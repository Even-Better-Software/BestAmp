using System.Net.Mime;
using System.Net.Sockets;
using System.Net.WebSockets;
using Microsoft.AspNetCore.Builder;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Hosting;
using System.Text.Json;

namespace BestAmp.App;

public static class MessageTranslator
{
    private const string ERR_NOT_WS = "Not a web socket request.";
    private const string ERR_NOT_JSON = "Received non-JSON object.";
    private const string ERR_CLOSE_FAILED = "Tried to close connection twice?";
    private const string ERR_NOT_BINARY = "Received web socket msg not in binary format.";


    public static readonly WebApplication WebApp;


    static MessageTranslator()
    {
        WebSocketOptions sockOptions = new()
        {
            KeepAliveInterval = TimeSpan.FromSeconds(Config.KEEP_ALIVE_INTERVAL),
        };

        WebApplicationBuilder builder = WebApplication.CreateBuilder();
        builder.WebHost.UseUrls([Config.BRIDGE_ADDRESS]);
        builder.Services.AddCors(options => options.AddPolicy(Config.CORS_POLICY, policy => policy
            .WithOrigins(Config.CLIENT_ORIGINS)
            .AllowAnyHeader()
            .AllowAnyMethod()
        ));

        WebApplication app = builder.Build();
        app.UseCors(Config.CORS_POLICY);
        app.UseWebSockets(sockOptions);

        // setup tcp/ws connection
        app.Map(Config.ENDPOINT_BACKEND, async context =>
        {
            if (!context.WebSockets.IsWebSocketRequest)
            {
                Console.WriteLine(ERR_NOT_WS);
                context.Response.StatusCode = StatusCodes.Status400BadRequest;
                return;
            }
            using WebSocket sock = await context.WebSockets.AcceptWebSocketAsync();

            using TcpClient tcp = new() { NoDelay = true };
            await tcp.ConnectAsync(Config.TCP_IP, Config.TCP_PORT, context.RequestAborted);
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

                AudioEngineProcess.CleanUpProcess();
                Environment.Exit(0);
            }
        });

        // return prefs as json, if file exists
        app.MapGet(Config.ENDPOINT_PREFS, async (CancellationToken cancellationToken) =>
        {
            string filePath = Config.UserPrefsPath;

            if (!File.Exists(filePath))
            {
                return Results.NotFound();
            }

            string json = await File.ReadAllTextAsync(filePath, cancellationToken);
            return Results.Content(json, MediaTypeNames.Application.Json);
        });

        // write to prefs file
        app.MapPut(Config.ENDPOINT_PREFS, async (JsonElement prefsJson, CancellationToken cancellationToken) =>
        {
            if (prefsJson.ValueKind != JsonValueKind.Object)
            {
                return Results.BadRequest(ERR_NOT_JSON);
            }

            string filePath = Config.UserPrefsPath;
            string directory = Path.GetDirectoryName(filePath)!;

            Directory.CreateDirectory(directory); // only runs if dir doesn't exist

            JsonSerializerOptions serializerOptions = new()
            {
                WriteIndented = true
            };
            string json = JsonSerializer.Serialize(prefsJson, serializerOptions);
            await File.WriteAllTextAsync(filePath, json, cancellationToken);

            return Results.NoContent();
        });

        WebApp = app;
    }

    /**
     * Listens for TCP traffic. Forwards it to the WebSocket connection.
     */
    static async Task ForwardTcpToBrowserAsync(
        NetworkStream tcpStream,
        WebSocket ws,
        CancellationToken cancelToken)
    {
        byte[] buffer = new byte[Config.BUFFER_SIZE];
        while (ws.State == WebSocketState.Open)
        {
            int bytesRead = await tcpStream.ReadAsync(buffer.AsMemory(0, Config.BUFFER_SIZE), cancelToken);

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
        byte[] buffer = new byte[Config.BUFFER_SIZE];
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
}
