# Toralize - SOCKS4 Proxy Library Makefile
CC = gcc
CFLAGS = -fPIC -shared -D_GNU_SOURCE -Wall -Wextra -O2
LDFLAGS = -ldl
TARGET = toralize.so
SOURCES = toralize.c

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(SOURCES) toralize.h
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET) test_client

test: $(TARGET) test_client
	@echo "Testing Toralize library..."
	@LD_PRELOAD=./$(TARGET) ./test_client example.com 80

# Simple test program
test_client: test_client.c
	$(CC) -o $@ test_client.c
