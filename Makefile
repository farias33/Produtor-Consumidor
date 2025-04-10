
# Makefile para o projeto Produtor-Consumidor

CC = gcc
CFLAGS = -Wall -O2
TARGET = produtor-consumidor
SRC = produtor_consumidor.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: all
	./$(TARGET) 2 2

clean:
	rm -f $(TARGET) buffer.txt buffer.txt.lock temp.txt
