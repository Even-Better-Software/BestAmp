using System.Diagnostics;

namespace BestAmp.App;

public static class AudioEngineProcess
{
    private static readonly string ERR_MISSING_WWWROOT = "Missing BestAmp front end UI files. Check your installation.";

    private static Process? _audioEngineProc;
    
    public static void SpawnProcess()
    {
        if (!Path.Exists(Config.AudioEnginePath))
        {
            Console.Error.WriteLine(ERR_MISSING_WWWROOT);
            Environment.Exit(1);
        }

        Process proc = new();
        proc.StartInfo.UseShellExecute = true;
        proc.StartInfo.FileName = Config.AudioEnginePath;
        proc.StartInfo.CreateNoWindow = false;
        proc.Start();

        Console.WriteLine("audio engine spawned successfully");

        _audioEngineProc = proc;
    }

    public static void CleanUpProcess()
    {
        _audioEngineProc?.Kill();
    }
}
