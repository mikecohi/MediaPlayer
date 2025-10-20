# =====================================================
# MediaPlayer - Makefile
# =====================================================

CXX := g++

# Compiler and Linker flags managed by pkg-config
# Requires: libncurses-dev, libsdl2-dev, libsdl2-mixer-dev, libtag1-dev
CXXFLAGS := -std=c++17 -Wall -g -I./src $(shell pkg-config --cflags ncurses sdl2 taglib)
LDFLAGS := $(shell pkg-config --libs ncurses sdl2 taglib SDL2_mixer)

# Project directories
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := $(SRC_DIR)/tests

# ---------------------------------------
# Collect sources and objects for the main app
# (Excludes test files)
# ---------------------------------------
SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp" ! -path "$(TEST_DIR)/*")
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Auto-generate dependency files (.d)
DEPS := $(OBJS:.o=.d)

# Main application executable
TARGET := $(BIN_DIR)/mediaplayer

# =======================================
# TARGETS
# =======================================

.PHONY: all test clean

# Default target: build the main application
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✅ Build complete: $(TARGET)"

# ---------------------------------------
# Unit test target
# ---------------------------------------
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/%.out, $(TEST_SRCS))

# Build and run all available unit tests
test: $(TEST_BINS)
	@echo "🧪 Running all unit tests..."
	@for t in $(TEST_BINS); do \
		if grep -q "int main" $$(echo $$t | sed 's|$(BIN_DIR)|$(TEST_DIR)|;s|.out|.cpp|'); then \
			echo "▶ Running $$t..."; \
			./$$t || exit 1; \
		else \
			echo "⚠️ Skipping $$t (no main)"; \
		fi; \
	done
	@echo "✅ All available tests executed!"

# Rule to build each test executable
# Skips building if 'int main' is not found
$(BIN_DIR)/%.out: $(TEST_DIR)/%.cpp $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) | $(BIN_DIR)
	@if grep -q "int main" $<; then \
		echo "🔧 Building test: $<"; \
		$(CXX) $(CXXFLAGS) $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $< -o $@ $(LDFLAGS); \
	else \
		echo "⚠️ Skipping $< (no main)"; \
	fi

# ---------------------------------------
# Generic rule to build object files
# -MMD -MP flags create dependency files
# ---------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# ---------------------------------------
# Folder creation
# ---------------------------------------
$(BIN_DIR) $(OBJ_DIR):
	@mkdir -p $@

# ---------------------------------------
# Clean build files
# ---------------------------------------
clean:
	@echo "🧹 Cleaning build files..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)

# ---------------------------------------
# Include auto-generated dependencies
# ---------------------------------------
-include $(DEPS)