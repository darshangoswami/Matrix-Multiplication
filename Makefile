# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -pthread

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

.PHONY: all clean debug