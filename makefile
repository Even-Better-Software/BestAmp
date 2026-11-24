.RECIPEPREFIX = >

BUILD_DIR = build
DIST_DIR = dist

NET_VERSION = net10.0
NET_VERSION_WIN = $(NET_VERSION)-windows

TARGET = $(BUILD_DIR)\BestAmp.exe

AE = ./BestAmp.AudioEngine
AE_BIN = BestAmp.AudioEngine.exe
AE_DP = $(AE)/build/*.dll
AE_CU = $(AE)/build/$(AE_BIN)

GUI = ./BestAmp.GUI
GUI_WWWROOT = $(GUI)/bin/Release/$(NET_VERSION)/publish/wwwroot/

APP = ./BestAmp.App

all: $(TARGET)


$(TARGET): 
> (cd $(AE); make; cd ../)
> (cd $(GUI); dotnet publish; cd ../)
> (cd $(APP); dotnet publish; cd ../)
> cp $(AE_DP) $(AE_CU) $(APP)/bin/Release/$(NET_VERSION_WIN)/
> cp $(GUI_WWWROOT) $(APP)/bin/Release/$(NET_VERSION_WIN)/ --recursive
