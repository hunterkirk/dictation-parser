CXX := clang++

CXXFLAGS := -std=c++20 -O3 -Wall -Wextra -Wpedantic
DEBUG_FLAGS := -std=c++20 -g -O0 -Wall -Wextra -Wpedantic

BUILD_DIR := build
BINARY := $(BUILD_DIR)/dictation-parser

SOURCES := \
	src/main.cpp \
	src/DictationParser.cpp

HEADERS := \
	src/DictationParser.h

.PHONY: all build intel apple universal clean rebuild run install debug help

# Default build
all: apple

build: apple

# Apple Silicon
apple: $(SOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -arch arm64 $(SOURCES) -o $(BINARY)
	@echo ""
	@echo "Built Apple Silicon binary:"
	@file $(BINARY)

# Intel Mac
intel: $(SOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -arch x86_64 $(SOURCES) -o $(BINARY)
	@echo ""
	@echo "Built Intel binary:"
	@file $(BINARY)

# Universal Intel + Apple Silicon
universal: $(SOURCES) $(HEADERS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -arch arm64 -arch x86_64 $(SOURCES) -o $(BINARY)
	@echo ""
	@echo "Built universal binary:"
	@file $(BINARY)

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean
	$(MAKE) build

debug:
	$(MAKE) clean
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(DEBUG_FLAGS) -arch arm64 $(SOURCES) -o $(BINARY)

run: build
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make run FILE=transcript.txt"; \
		exit 1; \
	fi
	$(BINARY) "$(FILE)"

install: build
	mkdir -p "$(HOME)/bin"
	cp "$(BINARY)" "$(HOME)/bin/dictation-parser"
	@echo ""
	@echo "Installed: $(HOME)/bin/dictation-parser"

help:
	@echo "dictation-parser"
	@echo ""
	@echo "  make                         Build Apple Silicon"
	@echo "  make apple                   Build arm64"
	@echo "  make intel                   Build x86_64"
	@echo "  make universal               Build arm64 + x86_64"
	@echo "  make clean                   Remove build directory"
	@echo "  make rebuild                 Clean and rebuild"
	@echo "  make debug                   Debug arm64 build"
	@echo "  make run FILE=file.txt       Run parser"
	@echo "  make install                 Install to ~/bin"
