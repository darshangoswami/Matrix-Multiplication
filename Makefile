CC=gcc
CFLAGS=-pthread -O2 -Wall
TARGET=matrix_mult

all: $(TARGET)

$(TARGET): matrix_mult.c
	$(CC) $(CFLAGS) -o $(TARGET) matrix_mult.c -lm

clean:
	rm -f $(TARGET)