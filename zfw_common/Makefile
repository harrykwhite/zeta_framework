.PHONY: all make_dirs

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

OBJS = $(OBJ_DIR)/zfw_common_debugging.o $(OBJ_DIR)/zfw_common_math.o

INCLUDE_PATHS = -Iinclude

all: make_dirs $(BIN_DIR)/libzfw_common.a

make_dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(OBJ_DIR)

$(BIN_DIR)/libzfw_common.a: $(OBJS)
	ar rcs $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	gcc $(COMP_FLAGS) -c $< -o $@ $(INCLUDE_PATHS)
