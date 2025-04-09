CC = gcc
CFLAGS = -Wall -Wextra -pedantic -ansi -g
LDFLAGS =

# Source and object directories
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include
BIN_DIR = bin

# Source files (explicitly list needed files to control compilation order)
SRCS = $(SRC_DIR)/main.c \
       $(SRC_DIR)/assembler.c \
       $(SRC_DIR)/preprocessor.c \
       $(SRC_DIR)/first_pass.c \
       $(SRC_DIR)/second_pass.c \
       $(SRC_DIR)/symbol_table.c \
       $(SRC_DIR)/instruction.c \
       $(SRC_DIR)/directive.c \
       $(SRC_DIR)/error_handler.c \
       $(SRC_DIR)/file_handler.c \
       $(SRC_DIR)/utils.c

OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))

# Header files
HEADERS = $(wildcard $(INC_DIR)/*.h)

# Target executable
TARGET = $(BIN_DIR)/assembler

# Default target
all: directories $(TARGET)

# Create directories if they don't exist
directories:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Linking the target executable
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

# Compiling source files with header dependencies
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -I$(INC_DIR) -c $< -o $@

# Clean up
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Test assembly of example files
test: all
	$(TARGET) example1

.PHONY: all clean test directories
