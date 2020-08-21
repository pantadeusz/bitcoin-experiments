LIBS=$(shell pkg-config --libs libbitcoin)
CFLAGS=$(shell pkg-config --cflags libbitcoin)


all: my_addr.o
	g++ -ggdb my_addr.o -o my_addr $(LIBS)

my_addr.o: my_addr.cpp
	g++  -ggdb $(CFLAGS) my_addr.cpp -c

clean:
	rm -f *.o my_addr
