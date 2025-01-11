.SUFFIXES:
.PHONY: all clean gdb build_dir test
all: build_dir $(TARGET)

SHELL=/bin/sh
CC=gcc
FLAGS=
BUILD_DIR=build
CFLAGS= -fPIC -g -pthread -std=c11
DEBUGFLAGS=-O0 -D _DEBUG -g
LDFLAGS= -shared -latomic
TEST_LDFLAGS= -L. $(BUILD_DIR)/alloc.so -latomic
DEP=.dependencies

TARGET=alloc.so
SOURCES=$(wildcard src/*.c)
TEST_SOURCES=$(wildcard test/*.c)
HEADERS=$(wildcard include/*.h)
OBJECTS=$(addprefix $(BUILD_DIR)/, $(notdir $(SOURCES:.c=.o)))
TEST_OBJECTS=$(addprefix $(BUILD_DIR)/, $(notdir $(TEST_SOURCES:.c=.o)))
DEPFILES=$(addprefix $(DEP)/, $(notdir $(SOURCES:.c=.d)))

all: build_dir $(TARGET)

clean:
	rm -f $(OBJECTS) $(TARGET) $(DEPFILES) $(TEST_OBJECTS) $(BUILD_DIR)/test_executable

build_dir:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(DEP)

test: build_dir $(TEST_OBJECTS)
	$(CC) $(TEST_OBJECTS) -o $(BUILD_DIR)/test_executable $(TEST_LDFLAGS) $(CFLAGS)
	./$(BUILD_DIR)/test_executable

$(BUILD_DIR)/%.o: test/%.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -MMD -MP -Iinclude -Itest -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BUILD_DIR)/$@ $(LDFLAGS)

$(BUILD_DIR)/%.o: src/%.c
	$(CC) $(CFLAGS) $(DEBUGFLAGS) -MMD -MP -Iinclude -c $< -o $@

-include $(DEPFILES)

debug: $(TARGET) $(TEST_OBJECTS)
	@gdb $(BUILD_DIR)/test_executable
	