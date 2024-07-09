.PHONY: all make_zfw_common make_output_dirs

DEBUG ?= 0

COMP_FLAGS := -Wall -std=c11

OUTPUT_SUBDIR := release

ifeq ($(DEBUG), 1)
    COMP_FLAGS := -g $(COMP_FLAGS)
    OUTPUT_SUBDIR := debug
endif

SRC_DIR := src
INCLUDE_DIRS := -Iinclude -I../zfw_common/include
LIB_DIRS := -L../zfw_common/bin/$(OUTPUT_SUBDIR)
OBJ_DIR := obj/$(OUTPUT_SUBDIR)
BIN_DIR := bin/$(OUTPUT_SUBDIR)

SRCS := $(SRC_DIR)/zfw_game.c $(SRC_DIR)/zfw_input.c $(SRC_DIR)/zfw_assets.c $(SRC_DIR)/zfw_rendering.c $(SRC_DIR)/zfw_utils.c $(SRC_DIR)/external/glad.c
LIBS := -lzfw_common
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: make_zfw_common make_output_dirs $(BIN_DIR)/libzeta_framework.a

make_zfw_common:
	$(MAKE) -C ../zfw_common

make_output_dirs:
	mkdir -p $(OBJ_DIR) $(OBJ_DIR)/external $(BIN_DIR)

$(BIN_DIR)/libzeta_framework.a: $(OBJS)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_DIRS)
