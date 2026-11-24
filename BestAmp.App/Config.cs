using System.Reflection;

namespace BestAmp.App;

public static class Config
{
    // ID of CORS policy w/ client
    public const string CORS_POLICY = "Default";

    // endpoints
    public const string ENDPOINT_BACKEND = "/backend";
    public const string ENDPOINT_PREFS = "/prefs";

    // file system locations
    public const string DIR_DATA = "BestAmp";
    public const string FILE_AUDIO_ENGINE = "BestAmp.exe";
    public const string FILE_PREFS = "ba_prefs.json";


    // address for non-TCP communication w/ the bridge
    public const string BRIDGE_ADDRESS = "http://127.0.0.1:7081";

    // address for TCP communication w/ the bridge
    public const string TCP_IP = "127.0.0.1";
    public const int TCP_PORT = 7080;

    // virtual host name
    public const string VHOST_NAME = "bestamp.test";

    // interval to send keep-alive frames to the websocket connection, in seconds 
    public const int KEEP_ALIVE_INTERVAL = 30;

    // max buffer size for receiving either a websocket or TCP message
    public const int BUFFER_SIZE = 65536;

    // any possible addresses where the client is reachable
    public static readonly string[] CLIENT_ORIGINS = [
        $"http://{VHOST_NAME}",
        $"https://{VHOST_NAME}"
    ];
    
    // Full paths
    public static string AudioEnginePath => Path.Combine(AppDir, FILE_AUDIO_ENGINE);
    public static string WwwrootPath => Path.Combine(AppDir, "wwwroot");
    public static string UserPrefsPath => Path.Combine(DataDir, FILE_PREFS);

    // path to app root directory
    private const string _ENV_APP_DIR = "BA_DDIR";
    private static string? _appDir;
    public static string AppDir { 
        get
        {
            _appDir ??= ResolveAppDir();
            return _appDir;
        } 
    }

    // path to data root directory
    private const string _ENV_USER_DIR = "BA_UDDIR";
    private static string? _dataDir;
    public static string DataDir {
        get 
        {
            _dataDir ??= ResolveUserDir();
            return _dataDir;
        } 
    }

    static Config()
    {
        Console.WriteLine($"Data directory {AppDir}");
        Console.WriteLine($"User data directory {DataDir}");
        Console.WriteLine($"Audio Engine: {AudioEnginePath}");
        Console.WriteLine($"GUI files directory: {WwwrootPath}");
    }

    private static string ResolveAppDir()
    {
        string assemDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location)!;

#if DEBUG
        assemDir = Path.GetFullPath(Environment.GetEnvironmentVariable(_ENV_APP_DIR) ?? assemDir);
#endif

        Directory.CreateDirectory(assemDir);
        return assemDir;
    }

    private static string ResolveUserDir()
    {
        string dir = "";

#if DEBUG
        dir = Path.GetFullPath(
            Environment.GetEnvironmentVariable(_ENV_USER_DIR) ?? Path.Combine(
                Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                DIR_DATA
            )
        );
#else
        dir = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            DIR_DATA
        );
#endif

        Directory.CreateDirectory(dir);
        return dir;
    }
}

