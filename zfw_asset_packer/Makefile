.PHONY: all make_dirs make_zfw_common

DEBUG ?= 0

COMP_FLAGS = -Wall -std=c11

ifeq ($(DEBUG), 1)
    COMP_FLAGS += -g

    BIN_SUBDIR = debug
    OBJ_SUBDIR = debug
else
    BIN_SUBDIR = release
    OBJ_SUBDIR = release
endif

SRC_DIR = src

OBJ_DIR = obj/$(OBJ_SUBDIR)
BIN_DIR = bin/$(BIN_SUBDIR)

OBJS = $(OBJ_DIR)/stb_image.o $(OBJ_DIR)/main.o

INCLUDE_PATHS = -I../zfw_common/include

LIB_PATHS = -L../zfw_common/bin/$(BIN_SUBDIR)
LIBS = -lzfw_common -lcjson

all: make_zfw_common make_dirs $(BIN_DIR)/zfw_asset_packer

make_zfw_common:
	$(MAKE) -C ../zfw_common DEBUG=$(DEBUG)

make_dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

$(BIN_DIR)/zfw_asset_packer: $(OBJS)
	gcc $(COMP_FLAGS) -o $@ $^ $(LIB_PATHS) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_PATHS)
