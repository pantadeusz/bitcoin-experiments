LIBS=$(shell pkg-config --libs libbitcoin)
CFLAGS=$(shell pkg-config --cflags libbitcoin)


all: txx

txx: txx.o
	g++ -ggdb txx.o -o txx $(LIBS)

txx.o: txx.cpp
	g++  -ggdb $(CFLAGS) txx.cpp -c

clean:
	rm -f *.o txx
