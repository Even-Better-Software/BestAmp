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

DLL_SRC     = external/portaudio/bin/libportaudio.dll
DLL_DEST    = $(BUILD_DIR)/libportaudio.dll

all: $(TARGET) $(DLL_DEST)


$(BUILD_DIR):
> mkdir -p $(BUILD_DIR)

$(BA_DSP_O): $(BA_DSP) | $(BUILD_DIR)
> $(CC) $(BA_DSP) -o $(BA_DSP_O) -c $(CFLAGS)

$(TARGET): $(MAIN) $(BA_DSP_O) | $(BUILD_DIR)
> $(CC) $(MAIN) $(BA_DSP_O) -o $@ $(CFLAGS) $(LDFLAGS)


$(DLL_DEST): $(DLL_SRC) | $(BUILD_DIR)
> cp $(DLL_SRC) $(DLL_DEST)

