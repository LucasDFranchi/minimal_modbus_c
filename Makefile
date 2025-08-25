# ----------------------------
# Project directories
# ----------------------------
SRC_DIR := src
INC_DIR := inc
TEST_DIR := test
BUILD_DIR := build

# ----------------------------
# Compiler and flags
# ----------------------------
CC := gcc
CFLAGS := -I$(INC_DIR) -Wall -Wextra -std=c11 -g -fprofile-arcs -ftest-coverage
LDFLAGS := -lcmocka -fprofile-arcs -ftest-coverage

# ----------------------------
# Source files
# ----------------------------
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

# ----------------------------
# Test files
# ----------------------------
TEST_SRCS := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJS := $(patsubst $(TEST_DIR)/%.c,$(BUILD_DIR)/%.test.o,$(TEST_SRCS))
TEST_BIN := $(BUILD_DIR)/run_tests

# ----------------------------
# Targets
# ----------------------------
.PHONY: all clean tests

all: $(BUILD_DIR) $(OBJS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------
# Build tests
# ----------------------------
tests: $(BUILD_DIR) $(TEST_OBJS) $(OBJS)
	$(CC) $(CFLAGS) $(TEST_OBJS) $(OBJS) -o $(TEST_BIN) $(LDFLAGS)
	@echo "[INFO] Built test binary: $(TEST_BIN)"

$(BUILD_DIR)/%.test.o: $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# ----------------------------
# Run tests
# ----------------------------
run_tests: tests
	@echo "[INFO] Running tests..."
	$(TEST_BIN)

# ----------------------------
# Coverage report
# ----------------------------
coverage: run_tests
	@echo "[INFO] Generating coverage report..."
	lcov --capture --directory . --output-file $(BUILD_DIR)/coverage.info
	lcov --remove $(BUILD_DIR)/coverage.info '/usr/*' 'test/*' --output-file $(BUILD_DIR)/coverage.info
	genhtml $(BUILD_DIR)/coverage.info --output-directory $(BUILD_DIR)/coverage_html
	@echo "[INFO] Coverage report ready: $(BUILD_DIR)/coverage_html/index.html"

# ----------------------------
# Clean build
# ----------------------------
clean:
	rm -rf $(BUILD_DIR)
