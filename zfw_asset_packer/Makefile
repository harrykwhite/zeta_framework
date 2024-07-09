.PHONY: all make_zfw_common make_output_dirs

DEBUG ?= 0

COMP_FLAGS := -Wall -std=c11

OUTPUT_SUBDIR := release

ifeq ($(DEBUG), 1)
    COMP_FLAGS := -g $(COMP_FLAGS)
    OUTPUT_SUBDIR := debug
endif

SRC_DIR := src
INCLUDE_DIRS := -I../zfw_common/include $(shell pkgconf --cflags freetype2)
LIB_DIRS := -L../zfw_common/bin/$(OUTPUT_SUBDIR) $(shell pkgconf --libs freetype2)
OBJ_DIR := obj/$(OUTPUT_SUBDIR)
BIN_DIR := bin/$(OUTPUT_SUBDIR)

SRCS := $(SRC_DIR)/main.c
LIBS := -lzfw_common -lcjson -lfreetype
OBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

all: make_zfw_common make_output_dirs $(BIN_DIR)/zfw_asset_packer

make_zfw_common:
	$(MAKE) -C ../zfw_common

make_output_dirs:
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(BIN_DIR)/zfw_asset_packer: $(OBJS)
	gcc $(COMP_FLAGS) -o $@ $^ $(INCLUDE_DIRS) $(LIB_DIRS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(COMP_FLAGS) $< -o $@ $(INCLUDE_DIRS)
