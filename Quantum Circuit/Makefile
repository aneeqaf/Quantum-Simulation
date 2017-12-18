TARGET = quantum
LIBS = -lm -lstdc++
CC = gcc-7
#/usr/local/gcc7/bin/
CFLAGS = -O3 -Wall -std=c++17 -march=native -Ofast -finline-functions
 # CFLAGS = -g3 -S -Wall -std=c++17

.PHONY: default all clean

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.cpp, %.o, $(wildcard *.cpp))
HEADERS = $(wildcard *.h)

%.o: %.cpp $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

# export LD_LIBRARY_PATH=/usr/local/gcc7/lib64