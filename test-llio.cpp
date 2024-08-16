#include <iostream>
#include <fstream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>

#include <mpi.h>

constexpr int WRITE_SIZE = 16;

static void* mapFile(char* fileName, int size, bool overwrite) {
    int flags = O_RDWR;
    if(overwrite) flags |= O_CREAT | O_TRUNC;

    int fd = open(fileName, flags, 0666);
    if(fd < 0) {
        std::cerr << "test-llio.cpp: ERROR: Coud not open file \"" << fileName << "\"\n";
        perror("Perror Output:");
        return nullptr;
    }

    int ret = ftruncate(fd, WRITE_SIZE);
    if(ret < 0) {
        std::cerr << "test-llio.cpp: ERROR: Coud not truncate file \"" << fileName << "\"\n";
        perror("Perror Output:");
        return nullptr;
    }
    void* outData = mmap(NULL, WRITE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd, 0);
    if(outData == MAP_FAILED) {
        std::cerr << "test-llio.cpp: ERROR: Coud not map file \"" << fileName << "\"\n";
        perror("Perror Output:");
        return nullptr;
    }
    close(fd);
    return outData;
}

int main(int argc, char** argv) {

    if(argc != 3) {
        std::cerr << "test-llio.cpp: ERROR: Incorrect argument count.\n";
        std::cerr << "Usage: test-llio <llio-folder> <output-file>\n";
        return 1;
    }

    // each outputs their rank to a shared file
    // rank 0 then reads all of the ranks and prints them
    MPI_Init(0, 0);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char* rootFileName = "%s/test.out.%02d";
    
    std::cout << "Rank " << rank << " is writing data out...\n";
    char fileName[256];
    sprintf(fileName, rootFileName, argv[1], rank);
    void* outData = mapFile(fileName, WRITE_SIZE, true);
    if(outData == nullptr) return 1;
    // memset(outData, rank, WRITE_SIZE);
    char* buffer[WRITE_SIZE];
    memset(buffer, rank, WRITE_SIZE);
    memcpy(outData, buffer, WRITE_SIZE);
    munmap(outData, WRITE_SIZE);
    std::cout << "Rank " << rank << " write complete.\n";

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        std::cout << "Rank " << rank << " is merging data to final output file \"" << argv[2] << "\"...\n";
        std::ofstream outFile{argv[2]};
        if(!outFile.good()) {
            std::cerr << "test-llio.cpp: ERROR: Error opening output file \"" << argv[2] << "\"\n";
            perror("Perror Output:");
            return 1;
        }

        for(int i = 0; i != size; ++i) {
            sprintf(fileName, rootFileName, argv[1], i);
            char buf[WRITE_SIZE];
            void* inData = mapFile(fileName, WRITE_SIZE, false);
            if(inData == nullptr) continue;
            memcpy(buf, inData, WRITE_SIZE);
            std::cout << "Copying data from file \"" << fileName << "\" " << buf << '\n';
            outFile.seekp(i * WRITE_SIZE);
            outFile.write(buf, WRITE_SIZE);
            munmap(inData, WRITE_SIZE);
        }
        std::cout << "Rank " << rank << " merge complete.\n";
    }

    return 0;
}
