CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pedantic
TARGET = tarea1
SOURCE = Tarea1.c


all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCE)

clean:
	rm -f $(TARGET)
run: $(TARGET)
 ./$(TARGET) test.txt palabrastest.txt
.PHONY: all clean
