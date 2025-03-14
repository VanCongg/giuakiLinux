CC = gcc
CFLAGS = -Wall -pthread
TARGET = canteen

all: $(TARGET)

$(TARGET): canteen.o
	$(CC) $(CFLAGS) -o $(TARGET) canteen.o

canteen.o: canteen.c
	$(CC) $(CFLAGS) -c canteen.c

clean:
	rm -f $(TARGET) *.o

run: $(TARGET)
	./$(TARGET)
