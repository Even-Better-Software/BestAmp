using System.Net;
using System.Diagnostics;
using System.Net.Mime;
using System.Net.Sockets;
using System.Net.WebSockets;
using System.Text.Json;

//**-------------------------------------------------------**//

// any possible addresses where the client is reachable
string[] CLIENT_ORIGINS = [
    "http://localhost:7082",
    "http://127.0.0.1:7082",
    "https://localhost:7083",
    "https://127.0.0.1:7083",
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

// errors
const string ERR_NOT_WS = "Not a web socket request.";
const string ERR_NOT_JSON = "Received non-JSON object.";
const string ERR_CLOSE_FAILED = "Tried to close connection twice?";
const string ERR_NOT_BINARY = "Received web socket msg not in binary format.";

//**-------------------------------------------------------**//

/*  BestAmp audio engine initialization
*/


// Resolve the audio engine binary path
// Spawn the audio engine
string EXPECTED_BA_DIR = Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles), "BestAmp");
string EXPECTED_BA_BIN = Path.Combine(
    EXPECTED_BA_DIR, "audio-engine", "BestAmp.exe");

Console.Out.WriteLine($"{EXPECTED_BA_DIR}");
Console.Out.WriteLine($"{EXPECTED_BA_BIN}");

if (!Path.Exists(EXPECTED_BA_BIN))
{
    Console.Error.WriteLine("Missing BestAmp binary check you installation");
    Environment.Exit(1);
}
Process myProcess = new Process();
myProcess.StartInfo.UseShellExecute = true;
myProcess.StartInfo.FileName = EXPECTED_BA_BIN;
myProcess.StartInfo.CreateNoWindow = false;
myProcess.Start();
Console.WriteLine("audio engine spawned successfully");


WebSocketOptions sockOptions = new()
{
    KeepAliveInterval = TimeSpan.FromSeconds(KEEP_ALIVE_INTERVAL),
};

WebApplicationBuilder builder = WebApplication.CreateBuilder(args);
builder.Services.AddCors(options => options.AddPolicy(CORS_POLICY, policy => policy
    .WithOrigins(CLIENT_ORIGINS)
    .AllowAnyHeader()
    .AllowAnyMethod()
));

WebApplication app = builder.Build();

app.UseCors(CORS_POLICY);
app.UseWebSockets(sockOptions);
app.UseDefaultFiles();
app.UseStaticFiles();

app.MapFallbackToFile("index.html");

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
        browserToTcp.Dispose();
        tcpToBrowser.Dispose();
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

app.Run(BRIDGE_ADDRESS);

//**-------------------------------------------------------**//

/**
 * Returns the absolute file path of where the preferences file is stored.
 */
static string GetPrefsFilePath() => Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
    DATA_DIR,
    PREFS_FILE
);

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


Console.WriteLine("terminating the audio engine");
myProcess.Kill();
