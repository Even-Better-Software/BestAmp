using System.Net;
using System.Diagnostics;
using System.Net.Mime;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Runtime.InteropServices;
using System.Drawing;
using System.Text.Json;
using Microsoft.Extensions.FileProviders;
using Microsoft.AspNetCore.StaticFiles;
using Microsoft.Web.WebView2.WinForms;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Hosting;
using Microsoft.Web.WebView2.Core;
using System.Reflection;

// any possible addresses where the client is reachable
string[] CLIENT_ORIGINS = [
    // "http://localhost:7081",
    // "http://127.0.0.1:7081",
    // "https://localhost:7081",
    // "https://127.0.0.1:7081",
    // "http://localhost:7082",
    // "http://127.0.0.1:7082",
    // "https://localhost:7083",
    // "https://127.0.0.1:7083",
    "http://bestamp.test",
    "https://bestamp.test"
];

// address for non-TCP communication w/ the bridge
const string BRIDGE_ADDRESS = "http://127.0.0.1:7081";

// address for TCP communication w/ the bridge
IPAddress TCP_IP = IPAddress.Loopback;
const int TCP_PORT = 7080;

// interval to send keep-alive frames to the websocket connection, in seconds 
const int KEEP_ALIVE_INTERVAL = 30;

// max buffer size for receiving either a websocket or TCP message
const int BUFFER_SIZE = 65536;

// ID of CORS policy w/ client
const string CORS_POLICY = "Default";

// endpoints
const string ENDPOINT_BACKEND = "/backend";
const string ENDPOINT_PREFS = "/prefs";

// file system locations
const string DATA_DIR = "BestAmp";
const string PREFS_FILE = "ba_prefs.json";
const string AUDIO_ENGINE_DIR = "audio-engine";
const string AUDIO_ENGINE_FILE = "BestAmp.exe";

// errors
const string ERR_NOT_WS = "Not a web socket request.";
const string ERR_NOT_JSON = "Received non-JSON object.";
const string ERR_CLOSE_FAILED = "Tried to close connection twice?";
const string ERR_NOT_BINARY = "Received web socket msg not in binary format.";
const string ERR_MISSING_EXE = "Missing BestAmp binary. Check your installation.";
const string ERR_MISSING_WWWROOT = "Missing BestAmp front end UI files. Check your installation.";

// UI Customization
const string WINDOW_NAME = "BestAmp";
const int WINDOW_WIDTH = 1200;
const int WINDOW_HEIGHT = 800;


#region APP-AENG

// Resolve the audio engine binary path
// Spawn the audio engine
// [WO] TODO, override the executable path for development purposes w/ an env var
string EXPECTED_BA_DIR = Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "BestAmp"
);
string EXPECTED_BA_BIN = Path.Combine(
    EXPECTED_BA_DIR, AUDIO_ENGINE_DIR, AUDIO_ENGINE_FILE
);

Console.Out.WriteLine(EXPECTED_BA_DIR);
Console.Out.WriteLine(EXPECTED_BA_BIN);

if (!Path.Exists(EXPECTED_BA_BIN))
{
    Console.Error.WriteLine(ERR_MISSING_EXE);
    Environment.Exit(1);
}

Process audioEngine = new();
audioEngine.StartInfo.UseShellExecute = true;
audioEngine.StartInfo.FileName = EXPECTED_BA_BIN;
audioEngine.StartInfo.CreateNoWindow = false;
audioEngine.Start();
Console.WriteLine("audio engine spawned successfully");


#endregion


#region APP-WEBSRV

// Webserver options
// Ammend this to server the compiled audio engine front-end from the distributed path
// [WO] override the wwwroot path from an environment variable for development purposes
string EXPECTED_BA_WWWROOT = Path.Combine(EXPECTED_BA_DIR, "wwwroot");
if (!Directory.Exists(EXPECTED_BA_WWWROOT))
{
    Console.Error.WriteLine(ERR_MISSING_WWWROOT);
    CleanUpProcesess(audioEngine);
    Environment.Exit(1);
}

WebSocketOptions sockOptions = new()
{
    KeepAliveInterval = TimeSpan.FromSeconds(KEEP_ALIVE_INTERVAL),
};

WebApplicationBuilder builder = WebApplication.CreateBuilder(args);
builder.WebHost.UseUrls([ BRIDGE_ADDRESS ]);
builder.Services.AddCors(options => options.AddPolicy(CORS_POLICY, policy => policy
    .WithOrigins(CLIENT_ORIGINS)
    .AllowAnyHeader()
    .AllowAnyMethod()
));

WebApplication app = builder.Build();
app.UseCors(CORS_POLICY);
app.UseWebSockets(sockOptions);
/*
app.UseStaticFiles(new StaticFileOptions
{
    FileProvider = new PhysicalFileProvider(EXPECTED_BA_WWWROOT),
    ContentTypeProvider = provider,
    ServeUnknownFileTypes = true
});
app.MapFallbackToFile("index.html", new StaticFileOptions
{
    FileProvider = new PhysicalFileProvider(EXPECTED_BA_WWWROOT),
    ServeUnknownFileTypes = true
});
*/

// setup tcp/ws connection
app.Map(ENDPOINT_BACKEND, async context =>
{
    if (!context.WebSockets.IsWebSocketRequest)
    {
        Console.WriteLine(ERR_NOT_WS);
        context.Response.StatusCode = StatusCodes.Status400BadRequest;
        return;
    }
    using WebSocket sock = await context.WebSockets.AcceptWebSocketAsync();

    using TcpClient tcp = new() { NoDelay = true };
    await tcp.ConnectAsync(IPAddress.Loopback, TCP_PORT, context.RequestAborted);
    using NetworkStream tcpStream = tcp.GetStream();

    using CancellationTokenSource cancelTokenSrc = 
            CancellationTokenSource.CreateLinkedTokenSource(context.RequestAborted);
    Task browserToTcp = ForwardBrowserToTcpAsync(sock, tcpStream, cancelTokenSrc.Token);
    Task tcpToBrowser = ForwardTcpToBrowserAsync(tcpStream, sock, cancelTokenSrc.Token);
    try
    {
        await Task.WhenAny(browserToTcp, tcpToBrowser);
    }
    catch (OperationCanceledException)
    {
        // this happens on a disconnect
        Console.Out.WriteLine("Connection closed cleanly.");
    }
    finally
    {
        cancelTokenSrc.Cancel();
        try
        {
            await sock.CloseAsync(WebSocketCloseStatus.NormalClosure, null, CancellationToken.None);
        }
        catch (Exception err)
        {
            Console.Error.WriteLine(ERR_CLOSE_FAILED);
            Console.Error.WriteLine(err);
        }
        browserToTcp.Dispose();
        tcpToBrowser.Dispose();

        CleanUpProcesess(audioEngine);
        Environment.Exit(0);
    }
});

// return prefs as json, if file exists
app.MapGet(ENDPOINT_PREFS, async (CancellationToken cancellationToken) =>
{
    string filePath = GetPrefsFilePath();

    if (!File.Exists(filePath))
    {
        return Results.NotFound();
    }

    string json = await File.ReadAllTextAsync(filePath, cancellationToken);
    return Results.Content(json, MediaTypeNames.Application.Json);
});

// write to prefs file
app.MapPut(ENDPOINT_PREFS, async (JsonElement prefsJson, CancellationToken cancellationToken) =>
{
    if (prefsJson.ValueKind != JsonValueKind.Object)
    {
        return Results.BadRequest(ERR_NOT_JSON);
    }

    string filePath = GetPrefsFilePath();
    string directory = Path.GetDirectoryName(filePath)!;

    Directory.CreateDirectory(directory); // only runs if dir doesn't exist

    JsonSerializerOptions serializerOptions = new() { 
        WriteIndented = true 
    }; 
    string json = JsonSerializer.Serialize(prefsJson, serializerOptions);
    await File.WriteAllTextAsync(filePath, json, cancellationToken);

    return Results.NoContent();
});

#endregion


#region APP-RT

Thread uiThread = new(() =>
{
    const int DWMWA_CAPTION_COLOR = 35;
    const int DWMWA_TEXT_COLOR = 36;

    Form form = new()
    {
        Text = WINDOW_NAME,
        Width = WINDOW_WIDTH,
        Height = WINDOW_HEIGHT,
        BackColor = Color.Black,
    };

    WebView2 webView = new()
    {
        BackColor = Color.Black,
        Dock = DockStyle.Fill
    };

    form.Icon = Icon.ExtractAssociatedIcon(Application.ExecutablePath);
    form.Controls.Add(webView);

    form.Load += async (sender, e) =>
    {
        Color c = Color.Black;
        int colorRef = c.R | (c.G << 8) | (c.B << 16);
        DwmSetWindowAttribute(form.Handle, DWMWA_CAPTION_COLOR, ref colorRef, sizeof(int));

        try
        {
            string ddir = GetDataDir();
            Directory.CreateDirectory(ddir);
            CoreWebView2Environment env = await CoreWebView2Environment.CreateAsync(userDataFolder: ddir);
            await webView.EnsureCoreWebView2Async(env);
            webView.CoreWebView2.SetVirtualHostNameToFolderMapping(
                "bestamp.test",
                EXPECTED_BA_WWWROOT,
                CoreWebView2HostResourceAccessKind.Allow
            );
            webView.CoreWebView2.Navigate("http://bestamp.test/index.html");
        }
        catch (Exception ex)
        {
            MessageBox.Show($"WebView2 initialization failed: {ex.Message}");
        }
    };
    Application.Run(form);
});


#endregion


#region APP-UTILS

[DllImport("dwmapi.dll", PreserveSig = true)]
static extern int DwmSetWindowAttribute(IntPtr hwnd, int attr, ref int attrValue, int attrSize);

static void CleanUpProcesess(Process? audioEngine = null, Process? renderer = null)
{
    audioEngine?.Kill(); 
    // renderer?.Kill();
}

/**
 * Returns the absolute file path of where the preferences file is stored.
 */
static string GetPrefsFilePath() => Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
    DATA_DIR,
    PREFS_FILE
);

static string GetDataDir() => Path.Combine(Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData), DATA_DIR).ToString();

/**
 * Listens for TCP traffic. Forwards it to the WebSocket connection.
 */
static async Task ForwardTcpToBrowserAsync(
    NetworkStream tcpStream, 
    WebSocket ws, 
    CancellationToken cancelToken)
{
    byte[] buffer = new byte[BUFFER_SIZE];
    while (ws.State == WebSocketState.Open)
    {
        int bytesRead = await tcpStream.ReadAsync(buffer.AsMemory(0, BUFFER_SIZE), cancelToken);

        if (bytesRead == 0)
        {
            Console.WriteLine("TCP peer closed the connection.");
            return;
        }

        await ws.SendAsync(buffer.AsMemory(0, bytesRead), WebSocketMessageType.Binary,
            endOfMessage: true, cancelToken
        );
    }
}

/**
 * Listens for WebSocket traffic. Forwards it to the TCP connection.
 */
static async Task ForwardBrowserToTcpAsync(WebSocket ws, NetworkStream tcpStream, 
    CancellationToken cancelToken)
{
    byte[] buffer = new byte[BUFFER_SIZE];
    while (ws.State == WebSocketState.Open)
    {
        WebSocketReceiveResult result = await ws.ReceiveAsync(
            buffer, cancelToken
        );

        if (result.MessageType == WebSocketMessageType.Close)
        {
            Console.Out.WriteLine("Websocket peer closed the connection.");
            return;
        }

        if (result.MessageType != WebSocketMessageType.Binary)
        {
            Console.Error.WriteLine(ERR_NOT_BINARY);
            continue;
        }

        await tcpStream.WriteAsync(buffer.AsMemory(0, result.Count), cancelToken);
    }
}

/**
 * Explicitly define mappings for important content types
 */
static FileExtensionContentTypeProvider MakeContentProvider()
{
    FileExtensionContentTypeProvider provider = new();
    
    provider.Mappings[".dat"] = "application/octet-stream";
    provider.Mappings[".blat"] = "application/octet-stream";
    provider.Mappings[".wasm"] = "application/wasm";
    provider.Mappings[".dll"] = "application/octet-stream";

    return provider;
}

#endregion


await app.StartAsync();

uiThread.SetApartmentState(ApartmentState.STA); 
uiThread.Start();
uiThread.Join();

await app.StopAsync();

CleanUpProcesess(audioEngine);

