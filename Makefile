CC      := gcc
CFLAGS  := -Wall -Wextra -std=c11 -O2 -D_POSIX_C_SOURCE=200809L
SRC_DIR := src
OBJ_DIR := obj
TARGET  := tarsau

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

.PHONY: all clean test

all: $(TARGET)

test: $(TARGET)
	@bash tests/run_tests.sh

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

-include $(DEPS)