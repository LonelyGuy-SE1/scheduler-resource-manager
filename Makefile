CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinclude
LDFLAGS = -pthread -lrt

SRC_DIR   = src
BUILD_DIR = build
INC_DIR   = include

SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/schedule.c \
       $(SRC_DIR)/logger.c \
       $(SRC_DIR)/ipc.c

OBJS   = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
TARGET = $(BUILD_DIR)/campuscompute

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(INC_DIR)/campus.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) campuscompute.log
