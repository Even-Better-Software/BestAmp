using System.Diagnostics;


namespace BestAmpUI.Bridge.BA_AEProc;

public static class BA_AEProc
{
    private static Process? aeproc = null;


    public static void SpawnProcess()
    {
        if (!Path.Exists(BA_Conf.BA_Conf.BA_AE_BIN))
        {
            Console.Error.WriteLine("Missing BestAmp binary. Check your installation.");
            Environment.Exit(1);
        }

        Process audioEngine = new();
        audioEngine.StartInfo.UseShellExecute = true;
        audioEngine.StartInfo.FileName = BA_Conf.BA_Conf.BA_AE_BIN;
        audioEngine.StartInfo.CreateNoWindow = false;
        audioEngine.Start();

        aeproc = audioEngine;

        Console.WriteLine("audio engine spawned successfully");
    }

    public static void CleanUpProcess()
    {
        aeproc?.Kill();
    }
}
