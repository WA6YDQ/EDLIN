all:	edlin.o	
	cc -o edlin edlin.o

static:	edlin.o
	cc -static -static-libgcc -static-libstdc++ -o edlin edlin.o

clean:
	rm *.o

install:
	sudo mv edlin /usr/local/bin/edlin

edlin.o:	edlin.c
		cc -c edlin.c


