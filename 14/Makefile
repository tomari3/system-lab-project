# Compiler and flags
CC = gcc
CFLAGS = -Wall -ansi -pedantic -g # -g for debugging symbols

# Directories
SRCDIR = src
INCDIR = include
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj

# Source files and object files
# Automatically finds all .c files in src/
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

# Executable name
TARGET = $(BUILDDIR)/assembler

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@
	@echo "Executable created: $@"

# Compile source files into object files
# Depends on the .c file and any .h file in include/
$(OBJDIR)/%.o: $(SRCDIR)/%.c $(wildcard $(INCDIR)/*.h)
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	rm -rf $(BUILDDIR) *.am # Also remove intermediate .am files

# Phony targets
.PHONY: all clean


