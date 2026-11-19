# BestAmpUI
Provides an intuitive user interface for the BestAmp audio engine.

## Running (Recommended)
Before running, ensure that you've installed the BestAmp audio engine such that BestAmp.exe and libportaudio.dll exist in your ProgramFiles/BestAmp/AudioEngine folder.
Then do the following:
1. `dotnet publish BestAmpUI.Client`
2. Copy the published wwwroot into the ProgramFiles/BestAmp folder (the one in the /publish folder)
3. `dotnet publish BestAmpUI.Bridge`
4. `"BestAmpUI.Bridge/bin/Release/net10.0/publish/BestAmpUI.Bridge.exe"`


## Running (Development)
1. `./BestAmp.exe` (in the directory where BestAmp.exe is installed)
2. `dotnet run --project BestAmpUI.Bridge`
3. `dotnet run --project BestAmpUI.Client`