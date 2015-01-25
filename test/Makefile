CC=gcc
OBJECT=
TARGET= test

INCLUDE=-I/usr/local/include -I/usr/include -I../
LIB= -lstdc++
WALL= -g -Wall

all: $(OBJECT) $(TARGET)

test: main.cpp
	$(CC) $(WALL) -o test main.cpp $(INCLUDE) $(LIB)

clean:
	rm -f *.lib *.o *.a $(TARGET) $(OBJECT)
