CC=../mpich-ins/bin/mpic++
# CC=mpiFCC

MERGER_INCLUDE_DIR="/vol0001/ra000004/keeganih/PMIO-LS/merger/"
MERGER_LIB_DIR="/vol0001/ra000004/keeganih/PMIO-LS/merger/out"

all: test-llio file-write

test-llio: test-llio.o
	${CC} -o test-llio test-llio.o

test-llio.o: test-llio.cpp
	${CC} -c test-llio.cpp

file-write: file-write.o
	${CC} -o file-write file-write.o -lpthread -L${MERGER_LIB_DIR} -lmergeThread

file-write.o: file-write.cpp
	${CC} -c file-write.cpp -I${MERGER_INCLUDE_DIR}

clean:
	rm *.o test-llio file-write
