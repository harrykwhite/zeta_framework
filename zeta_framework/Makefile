.PHONY: all make_zfw_common make_output_dirs

DEBUG ?= 0

COMP_FLAGS := -Wall -std=c11

OUTPUT_SUBDIR := release

ifeq ($(DEBUG), 1)
    COMP_FLAGS := -g $(COMP_FLAGS) -DZFW_DEBUG
    OUTPUT_SUBDIR := debug
endif

SRC_DIR := src
INCLUDE_DIRS := -Iinclude -I../zfw_common/include
LIB_DIRS := -L../zfw_common/bin/$(OUTPUT_SUBDIR)
OBJ_DIR := obj/$(OUTPUT_SUBDIR)
BIN_DIR := bin/$(OUTPUT_SUBDIR)

LIBS := -lzfw_common
OBJS := $(OBJ_DIR)/zfw_game.o $(OBJ_DIR)/zfw_input.o $(OBJ_DIR)/zfw_assets.o $(OBJ_DIR)/zfw_rendering.o $(OBJ_DIR)/zfw_utils.o $(OBJ_DIR)/external/glad.o

all: make_zfw_common make_output_dirs $(BIN_DIR)/libzeta_framework.a

make_zfw_common:
	$(MAKE) -C ../zfw_common

make_output_dirs: make_zfw_common
	mkdir -p $(OBJ_DIR) $(OBJ_DIR)/external $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | make_output_dirs
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_DIRS)

$(BIN_DIR)/libzeta_framework.a: $(OBJS)
	ar rcs $@ $^
