# =====================================================
# MediaPlayer - Full Makefile
# =====================================================

# --- Compiler setup ---
CXX := g++
SHELL := /bin/bash
CXXFLAGS := -std=c++17 -Wall -g \
    -I./src -I./src/external \
    $(shell pkg-config --cflags ncursesw sdl2 SDL2_mixer taglib)
LDFLAGS := $(shell pkg-config --libs ncursesw sdl2 SDL2_mixer taglib)

# --- Directory layout ---
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := $(SRC_DIR)/tests
INSTALL_DIR := /usr/local/bin

# --- Main target ---
TARGET := $(BIN_DIR)/mediaplayer

# =====================================================
# Collect source and object files
# =====================================================
SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp" ! -path "$(TEST_DIR)/*")
OBJS := $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

# =====================================================
# Default target
# =====================================================
.PHONY: all build run test install uninstall clean rebuild

# -----------------------------------------------------
# Build main executable
# -----------------------------------------------------
all: $(TARGET)

build: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "🔧 Linking $(TARGET)..."
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✅ Build complete: $(TARGET)"

# -----------------------------------------------------
# Object file compilation with dependency tracking
# -----------------------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# -----------------------------------------------------
# Directory creation rules
# -----------------------------------------------------
$(OBJ_DIR) $(BIN_DIR):
	@mkdir -p $@

# =====================================================
# Run the main app
# =====================================================
run: $(TARGET)
	@echo "🚀 Starting MediaPlayer..."
	@./$(TARGET)

# =====================================================
# Unit Tests
# =====================================================
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp,$(BIN_DIR)/%.out,$(TEST_SRCS))

test: $(TEST_BINS)
	@echo "🧪 Running all available tests..."
	@for t in $(TEST_BINS); do \
		echo "▶ Running $$t..."; \
		./$$t || exit 1; \
	done
	@echo "✅ All tests completed successfully!"

$(BIN_DIR)/%.out: $(TEST_DIR)/%.cpp $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) | $(BIN_DIR)
	@echo "🔧 Building test: $<"
	$(CXX) $(CXXFLAGS) $(filter-out $(OBJ_DIR)/main.o,$(OBJS)) $< -o $@ $(LDFLAGS)

# =====================================================
# Installation / Uninstallation
# =====================================================
install: $(TARGET)
	@echo "🔐 Installing requires administrator privileges..."
	sudo install -m 755 $(TARGET) $(INSTALL_DIR)/mediaplayer
	@echo "✅ Installed binary to $(INSTALL_DIR)/mediaplayer"

	@echo "🎵 Setting up user Music folders..."
	mkdir -p $$HOME/Music/MediaPlayer/test_media
	mkdir -p $$HOME/Music/MediaPlayer/playlist
	@echo "✅ User media folders ready at $$HOME/Music/MediaPlayer/"

uninstall:
	@echo "🗑️  Removing mediaplayer from $(INSTALL_DIR)..."
	sudo rm -f $(INSTALL_DIR)/mediaplayer
	@echo "✅ Uninstalled."

# =====================================================
# Clean & Rebuild
# =====================================================
clean:
	@echo "🧹 Cleaning object and binary files..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "✅ Clean done."

rebuild: clean all
	@echo "🔁 Rebuild complete."

# =====================================================
# Include dependency files
# =====================================================
-include $(DEPS)
