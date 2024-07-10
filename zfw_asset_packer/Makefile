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

LIBS := -lzfw_common -lcjson -lfreetype
OBJS := $(OBJ_DIR)/main.o

all: make_zfw_common make_output_dirs $(BIN_DIR)/zfw_asset_packer

make_zfw_common:
	$(MAKE) -C ../zfw_common

make_output_dirs: make_zfw_common
	mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | make_output_dirs
	gcc $(COMP_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

$(BIN_DIR)/zfw_asset_packer: $(OBJS)
	gcc $(COMP_FLAGS) $^ -o $@ $(LIB_DIRS) $(LIBS)
