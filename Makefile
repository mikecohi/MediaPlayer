# =====================================================
# MediaPlayer - Makefile
# Build main app + unit tests
# =====================================================

CXX := g++
CXXFLAGS := -std=c++17 -Wall -I./src
LDFLAGS := -lncurses -lSDL2 -ltag

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
TEST_DIR := $(SRC_DIR)/tests

# ---------------------------------------
# Collect all cpp files for the main app
# ---------------------------------------
SRCS := $(shell find $(SRC_DIR) -type f -name "*.cpp" ! -path "$(SRC_DIR)/tests/*")
OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# ---------------------------------------
# Collect all test files
# ---------------------------------------
TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJS := $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(TEST_SRCS))
TEST_BIN := $(BIN_DIR)/run_tests

# ---------------------------------------
# Main executable
# ---------------------------------------
TARGET := $(BIN_DIR)/mediaplayer

# ---------------------------------------
# Default target
# ---------------------------------------
all: $(TARGET)

$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "✅ Build complete: $@"

# ---------------------------------------
# Unit test target (build + run all tests)
# ---------------------------------------

TEST_SRCS := $(wildcard $(TEST_DIR)/*.cpp)
TEST_BINS := $(patsubst $(TEST_DIR)/%.cpp, $(BIN_DIR)/%.out, $(TEST_SRCS))

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

$(BIN_DIR)/%.out: $(TEST_DIR)/%.cpp $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) | $(BIN_DIR)
	@if grep -q "int main" $<; then \
		echo "🔧 Building test: $<"; \
		$(CXX) $(CXXFLAGS) $(filter-out $(OBJ_DIR)/main.o, $(OBJS)) $< -o $@ $(LDFLAGS); \
	else \
		echo "⚠️ Skipping $< (no main)"; \
	fi
# ---------------------------------------
# Object file rule
# ---------------------------------------
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ---------------------------------------
# Folder creation
# ---------------------------------------
$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

# ---------------------------------------
# Clean build files
# ---------------------------------------
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "🧹 Clean complete."

.PHONY: all test clean
