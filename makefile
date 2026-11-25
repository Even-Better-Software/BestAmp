.RECIPEPREFIX = >

NET = dotnet
NET_FLAGS =
NET_FLAGS_O = $(NET_FLAGS)
NET_VERSION = net10.0
NET_VERSION_WIN = $(NET_VERSION)-windows


TARGET = $(BUILD_DIR)\BestAmp.exe
BDIR =


AE = ./BestAmp.AudioEngine
AE_ARGS =
AE_BIN = BestAmp.AudioEngine.exe
AE_DP = $(AE)/bin/*.dll
AE_CU = $(AE)/bin/$(AE_BIN)

GUI = ./BestAmp.GUI
GUI_WWWROOT = $(GUI)/bin/Release/$(NET_VERSION)/publish/wwwroot/

APP = ./BestAmp.App

DIST = ./dist



all: NET_FLAGS += publish
all: BDIR = Release
all: audio-engine gui app run

publish: NET_FLAGS += publish
publish: BDIR = Release
publish: audio-engine gui app publish

debug: NET_FLAGS += build --configuration Debug
debug: AE_ARGS += debug
debug: BDIR = Debug
debug: audio-engine gui app run-debug


audio-engine:
> (cd $(AE); make $(AE_ARGS); cd ../)

gui:
> (cd $(GUI); $(NET) publish; cd ../)

app: audio-engine gui
> (cd $(APP); $(NET) $(NET_FLAGS); cd ../)
> cp $(AE_DP) $(AE_CU) $(APP)/bin/$(BDIR)/$(NET_VERSION_WIN)/
> cp $(GUI_WWWROOT) $(APP)/bin/$(BDIR)/$(NET_VERSION_WIN)/ --recursive



run:
> $(APP)/bin/Release/$(NET_VERSION_WIN)/BestAmp.App.exe

run-debug:
> $(APP)/bin/Debug/$(NET_VERSION_WIN)/BestAmp.App.exe

clean:
> echo "this does nothing rn"
# > (cd $(AE); make clean; cd ../)
# > (echo $(GUI)/bin/*; rm $(GUI)/bin/* -rfI)
# > (echo $(APP)/bin/*; rm $(APP)/bin/* -rfI)

publish: $(DIST)
> cp $(APP)/bin/Release/$(NET_VERSION_WIN)/* $(DIST)/ --recursive

clean-publish: $(DIST)
> echo "$(DIST)/*"
> rm $(DIST)/* -rfI

$(DIST):
> mkdir -p $(DIST)
