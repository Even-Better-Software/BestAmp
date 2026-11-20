using BestAmpUI.Bridge.BA_AEProc;
using BestAmpUI.Bridge.BA_Renderer;
using BestAmpUI.Bridge.BA_Msgtrnsl;

// BestAmp
try
{
    BA_AEProc.SpawnProcess();
    await BA_Msgtrnsl.BA_WS.StartAsync();

    BA_Renderer.BA_UI_THREAD.SetApartmentState(ApartmentState.STA); 
    BA_Renderer.BA_UI_THREAD.Start();
    BA_Renderer.BA_UI_THREAD.Join();
} finally
{
    await BA_Msgtrnsl.BA_WS.StopAsync();
    BA_AEProc.CleanUpProcess();
}

