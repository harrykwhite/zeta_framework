.PHONY: all make_output_dirs

DEBUG ?= 0

COMP_FLAGS := -Wall -std=c11

OUTPUT_SUBDIR := release

ifeq ($(DEBUG), 1)
	COMP_FLAGS := -g $(COMP_FLAGS)
	OUTPUT_SUBDIR := debug
endif

SRC_DIR := src
INCLUDE_DIRS := -Iinclude
OBJ_DIR := obj/$(OUTPUT_SUBDIR)
BIN_DIR := bin/$(OUTPUT_SUBDIR)

SRCS := $(SRC_DIR)/zfw_common_debug.c $(SRC_DIR)/zfw_common_mem.c $(SRC_DIR)/zfw_common_math.c
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: make_output_dirs $(BIN_DIR)/libzfw_common.a

make_output_dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(BIN_DIR)/libzfw_common.a: $(OBJS)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_DIRS)
