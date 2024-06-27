TARGET = tensor

CC = gcc
CFLAGS = -Wall -Wextra -pthread -std=c11

SRCS = tensor.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)