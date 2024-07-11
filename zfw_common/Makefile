.PHONY: all make_output_dirs

DEBUG ?= 0

COMP_FLAGS := -Wall -std=c11

OUTPUT_SUBDIR := release

ifeq ($(DEBUG), 1)
	COMP_FLAGS := -g $(COMP_FLAGS) -DZFW_DEBUG
	OUTPUT_SUBDIR := debug
endif

SRC_DIR := src
INCLUDE_DIRS := -Iinclude
OBJ_DIR := obj/$(OUTPUT_SUBDIR)
BIN_DIR := bin/$(OUTPUT_SUBDIR)

OBJS := $(OBJ_DIR)/zfw_common_debug.o $(OBJ_DIR)/zfw_common_mem.o $(OBJ_DIR)/zfw_common_math.o

all: make_output_dirs $(BIN_DIR)/libzfw_common.a

make_output_dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | make_output_dirs
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_DIRS)

$(BIN_DIR)/libzfw_common.a: $(OBJS)
	ar rcs $@ $^
