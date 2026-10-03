.RECIPEPREFIX = >

CC      = gcc
CFLAGS  = -Wall -Wextra -Iinclude -Iexternal/portaudio/include

LDFLAGS = -Lexternal/portaudio/lib -lportaudio

BUILD_DIR   = build
TARGET      = $(BUILD_DIR)/BestAmp.exe

SRC_DIR     = src
MAIN        = $(SRC_DIR)/main.c

BA_DSP      = $(SRC_DIR)/ba_dsp.c
BA_DSP_O    = $(BUILD_DIR)/ba_dsp.o
BA_AMP      = $(SRC_DIR)/ba_amp.c
BA_AMP_O    = $(BUILD_DIR)/ba_amp.o
BA_PREFS    = $(SRC_DIR)/ba_prefs.c
BA_PREFS_O  = $(BUILD_DIR)/ba_prefs.o
BA_PORTAUDIO_HELPERS = $(SRC_DIR)/ba_portaudio_helpers.c
BA_PORTAUDIO_HELPERS_O = $(BUILD_DIR)/ba_portaudio_helpers.o

DLL_SRC     = external/portaudio/bin/libportaudio.dll
DLL_DEST    = $(BUILD_DIR)/libportaudio.dll


all: $(TARGET) $(DLL_DEST)


$(BUILD_DIR):
> mkdir -p $(BUILD_DIR)


$(BA_DSP_O): $(BA_DSP) | $(BUILD_DIR)
> $(CC) $(BA_DSP) -o $(BA_DSP_O) -c $(CFLAGS)

$(BA_AMP_O): $(BA_AMP) | $(BUILD_DIR)
> $(CC) $(BA_AMP) -o $(BA_AMP_O) -c $(CFLAGS)

$(BA_PREFS_O): $(BA_PREFS) | $(BUILD_DIR)
> $(CC) $(BA_PREFS) -o $(BA_PREFS_O) -c $(CFLAGS)

$(BA_PORTAUDIO_HELPERS_O): $(BA_PORTAUDIO_HELPERS) | $(BUILD)
> $(CC) $(BA_PORTAUDIO_HELPERS) -o $(BA_PORTAUDIO_HELPERS_O) -c $(CFLAGS)


$(TARGET): $(MAIN) $(BA_DSP_O) $(BA_AMP_O) $(BA_PREFS_O) $(BA_PORTAUDIO_HELPERS_O) | $(BUILD_DIR)
> $(CC) $(MAIN) $(BA_DSP_O) $(BA_AMP_O) $(BA_PREFS_O) $(BA_PORTAUDIO_HELPERS_O) -o $@ $(CFLAGS) $(LDFLAGS)


$(DLL_DEST): $(DLL_SRC) | $(BUILD_DIR)
> copy $(DLL_SRC) $(DLL_DEST)

