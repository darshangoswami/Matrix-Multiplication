# Compiler and flags
CC = gcc
CFLAGS = -O3 -march=native -mtune=native -ffast-math -Wall -Wextra
LDFLAGS = -pthread -lm

# Target executable name
TARGET = matrix_mult

# Source files
SRC = matrix_mult.c

# Object files
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Linking the program
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compiling source files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(TARGET) $(OBJ)

# Debug build
debug: CFLAGS += -g -DDEBUG
debug: clean all

# Help target
help:
	@echo "Available targets:"
	@echo "  all     : Build the matrix multiplication program (default)"
	@echo "  clean   : Remove built files"
	@echo "  debug   : Build with debugging symbols"
	@echo "  help    : Show this help message"

.PHONY: all clean debug help