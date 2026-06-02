CC = gcc
CFLAGS = -O2 -std=c11 -Wall -Wextra -pedantic
SRC = src/main.c src/dmst.c src/heap.c src/fibheap.c src/unionfind.c
OBJ = $(SRC:.c=.o)
TARGET = dmst

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
