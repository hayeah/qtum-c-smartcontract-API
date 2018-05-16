TARGET = qtum.dylib
LIBS = -lleveldb
CC = cc
CFLAGS = -g -Wall -fPIC

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
		$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
		$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

qtum.dylib: $(OBJECTS)
	$(CC) $(CFLAGS) $(LIBS) -dynamiclib -o $@ $^

clean:
		rm -f *.o
		rm -f $(TARGET)
