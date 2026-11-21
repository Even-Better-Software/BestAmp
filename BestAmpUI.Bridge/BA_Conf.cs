using System;
using System.Net;
using System.Reflection;

namespace BestAmpUI.Bridge.BA_Conf;

public static class BA_Conf
{

    // any possible addresses where the client is reachable
    public static readonly string[] CLIENT_ORIGINS = [
        "http://bestamp.test",
        "https://bestamp.test"
    ];


    // ID of CORS policy w/ client
    public static readonly string CORS_POLICY = "Default";

    // endpoints
    public static readonly string ENDPOINT_BACKEND = "/backend";
    public static readonly string ENDPOINT_PREFS = "/prefs";

    // file system locations
    public static readonly string DATA_DIR = "BestAmp";
    public static readonly string PREFS_FILE = "ba_prefs.json";


    // address for non-TCP communication w/ the bridge
    public static readonly string BRIDGE_ADDRESS = "http://127.0.0.1:7081";

    // address for TCP communication w/ the bridge
    public static readonly IPAddress TCP_IP = IPAddress.Loopback;
    public static readonly int TCP_PORT = 7080;

    // interval to send keep-alive frames to the websocket connection, in seconds 
    public static readonly int KEEP_ALIVE_INTERVAL = 30;

    // max buffer size for receiving either a websocket or TCP message
    public static readonly int BUFFER_SIZE = 65536;


    private static readonly string _ebdirenv = "BA_DDIR";
    private static readonly string _ebudirenv = "BA_UDDIR";
    public static string BA_DIR { get; private set; }
    public static string BA_USER_DIR { get; private set; }

    public static string BA_AE_BIN { get { return Path.Combine(
        BA_DIR, "BestAmp.exe"
    ); }} // bin
    public static string BA_AE_GUI { get { return Path.Combine(
        BA_DIR, "wwwroot"
    ); }} // dir
    public static string BA_PREFS { get { return Path.Combine(
        BA_USER_DIR, "ba_prefs.json"
    ); } }


    static BA_Conf()
    {
        BA_DIR      = _res_dir();
        BA_USER_DIR = _res_u_dir();


        Console.WriteLine($"Data directory {BA_DIR}");
        Console.WriteLine($"User data directory {BA_USER_DIR}");
        Console.WriteLine($"Audio Engine: {BA_AE_BIN}");
        Console.WriteLine($"GUI files directory: {BA_AE_GUI}");
    }

    private static string _res_dir()
    {
        string assem = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
        string r = assem;
#if DEBUG
        r = Path.GetFullPath(Environment.GetEnvironmentVariable(_ebdirenv) ?? assem);
#endif
        Directory.CreateDirectory(r);
        return r;
    }

    private static string _res_u_dir()
    {
        string r = "";
#if DEBUG
        r = Path.GetFullPath(Environment.GetEnvironmentVariable(_ebudirenv) ?? Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "BestAmp"
        ));
#else
        r = Path.Combine(
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
            "BestAmp"
        );
#endif
        Directory.CreateDirectory(r);
        return r;
    }
}

