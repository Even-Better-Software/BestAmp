.RECIPEPREFIX = >

CC      = gcc
CFLAGS  = -Mj -Wall -Wextra -Iinclude -Iexternal/portaudio/include

LDFLAGS = -Lexternal/portaudio/lib -lportaudio

BUILD_DIR   = build
TARGET      = $(BUILD_DIR)/BestAmp.exe
SRC_DIR     = src
MAIN        = $(SRC_DIR)/main.c

DLL_SRC     = external/portaudio/bin/libportaudio.dll
DLL_DEST    = $(BUILD_DIR)/libportaudio.dll

all: $(TARGET) $(DLL_DEST)

$(TARGET): $(MAIN) | $(BUILD_DIR)
> $(CC) $(MAIN) -o $@ $(CFLAGS) $(LDFLAGS)

$(DLL_DEST): $(DLL_SRC) | $(BUILD_DIR)
> cp $(DLL_SRC) $(DLL_DEST)

$(BUILD_DIR):
> mkdir -p $(BUILD_DIR)

