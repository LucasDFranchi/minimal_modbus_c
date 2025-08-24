CC = gcc
CFLAGS = -Wall -Wextra -Iinc -g
LDFLAGS = 

SRC_DIR = src
INC_DIR = inc
BUILD_DIR = build

SRCS = $(SRC_DIR)/modbus_master.c $(SRC_DIR)/modbus_slave.c $(SRC_DIR)/modbus_utils.c
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

EXE = example
EXE_SRC = example.c

all: $(EXE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(EXE): $(EXE_SRC) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(EXE_SRC) $(OBJS) $(LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(EXE)
