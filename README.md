# BestAmpUI
Provides an intuitive user interface for the BestAmp audio engine.

## Running (Recommended)
Before running, ensure that you've installed the BestAmp audio engine such that BestAmp.exe and libportaudio.dll exist in your ProgramFiles/BestAmp folder.
Then do the following:
1. `dotnet publish BestAmpUI.Client`
2. Copy the published wwwroot into the ProgramFiles/BestAmp folder (the one in the /publish folder)
3. `dotnet publish BestAmpUI.Bridge`
4. Copy the published bridge folder into the ProgramFiles/BestAmp folder (everything in the /publish folder)
5. Run `BestAmpUI.Bridge.exe`


## Running (Development)
1. `BestAmp.exe` (in the directory where BestAmp.exe is installed)
2. `dotnet build`
3. `dotnet run --project BestAmpUI.Bridge`

