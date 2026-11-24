using BestAmp.App;

// BestAmp
try
{
    AudioEngineProcess.SpawnProcess();
    await MessageTranslator.WebApp.StartAsync();

    Renderer.UIThread.SetApartmentState(ApartmentState.STA); 
    Renderer.UIThread.Start();
    Renderer.UIThread.Join();
} finally
{
    await MessageTranslator.WebApp.StopAsync();
    AudioEngineProcess.CleanUpProcess();
}

