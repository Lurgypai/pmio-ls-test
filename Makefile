CC=../mpich-ins/bin/mpic++

all: test-llio

test-llio: test-llio.o
	${CC} -o test-llio test-llio.o

test-llio.o: test-llio.cpp
	${CC} -c test-llio.cpp

clean:
	rm *.o test-llio
