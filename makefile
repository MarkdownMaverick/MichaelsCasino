# Michael's Casino – Cross-Platform Makefile
# Works on: Linux (Ubuntu, Manjaro), macOS, Windows (MSYS2/MinGW)

# ── Detect OS ───────────────────────────────────────────────
UNAME_S := $(shell uname -s)
UNAME_M := $(shell uname -m)

# Set OS type
ifeq ($(UNAME_S),Linux)
    OS := LINUX
    LIBS_EXTRA := -lX11 -ldl -lpthread -lrt
    EXE :=
endif

ifeq ($(UNAME_S),Darwin)
    OS := MACOS
    LIBS_EXTRA := -framework Cocoa -framework OpenGL
    EXE :=
endif

# Windows detection (via MSYS2/MinGW)
ifeq ($(OS_TYPE),Windows_NT)
    OS := WINDOWS
    LIBS_EXTRA := -lws2_32
    EXE := .exe
endif

# Fallback for Windows detection
ifeq ($(OS),)
    ifneq (,$(findstring MINGW,$(shell uname)))
        OS := WINDOWS
        LIBS_EXTRA := -lws2_32
        EXE := .exe
    endif
endif

# ── Compiler & Flags ────────────────────────────────────────
CC := gcc
#CFLAGS := -Wall -Wextra -std=c99 -O2 -march=native -pipe

CFLAGS := -Wall -Wextra -Wpedantic -std=c99 -O0 -g \
          -Wunused -Wunused-variable -Wunused-parameter -Wunused-function \
          -Wshadow -Wcast-align -Wwrite-strings -Wstrict-prototypes \
          -Wmissing-prototypes -Wredundant-decls -Wnested-externs \
          -Wfloat-equal -Wswitch-default -Wswitch-enum \
          -Wconversion -Wformat=2 -Werror
# Directories
SRC_DIR := src
INC_DIR := include
OBJ_DIR := obj
LIB_DIR := lib

# Include paths
INCLUDES := -I$(INC_DIR) \
            -I$(LIB_DIR)/raylib/include \
            -I$(LIB_DIR)/cjson/include

# Library paths
LIB_PATHS := -L$(LIB_DIR)/raylib/lib \
             -L$(LIB_DIR)/cjson/lib

# Libraries to link
LIBS := $(LIB_DIR)/raylib/lib/libraylib.a \
        $(LIB_DIR)/cjson/lib/libcjson.a \
        -lm $(LIBS_EXTRA)

# Source & Object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
TARGET := Michaels_Casino$(EXE)

# ── Build Rules ─────────────────────────────────────────────
.PHONY: all clean run info install-deps

all: $(TARGET)
	@echo "✅ Build complete for $(OS)!"

$(TARGET): $(OBJ_DIR) $(OBJS)
	@echo "🔗 Linking for $(OS)..."
	$(CC) $(OBJS) $(LIB_PATHS) $(LIBS) -o $(TARGET)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "🔨 Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	@echo "🧹 Cleaning..."
	rm -rf $(OBJ_DIR) $(TARGET)

run: $(TARGET)
	@echo "🎮 Starting game on $(OS)..."
	./$(TARGET)

info:
	@echo "OS detected: $(OS)"
	@echo "Architecture: $(UNAME_M)"
	@echo "Executable: $(TARGET)"

install-deps:
	@echo "Installing dependencies for $(OS)..."
ifeq ($(OS),LINUX)
	sudo apt update && sudo apt install -y libraylib-dev libjson-c-dev build-essential
else ifeq ($(OS),MACOS)
	brew install raylib json-c
else ifeq ($(OS),WINDOWS)
	@echo "Install via MSYS2: pacman -S mingw-w64-x86_64-raylib mingw-w64-x86_64-json-c"
else
	@echo "❌ Unsupported OS: $(OS)"
endif