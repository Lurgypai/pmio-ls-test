#include <string>
#include <thread>

#include <cstring>

#include <mpi.h>

#include "../merger/mChunk.h"
#include "../merger/mergeThread.h"

#define IO_SIZE 4
#define IO_COUNT 256

int main(int argc, char *argv[])
{
    MPI_File fh;
    int rank;
    int size;
    MPI_Init(0, 0);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Info finfo;
    MPI_Info_create(&finfo);

    const char* do_coll_buffering = "enable";
    MPI_Info_set(finfo, "romio_cb_write", do_coll_buffering);
    MPI_Info_set(finfo, "romio_cb_read", do_coll_buffering);
    MPI_Info_set(finfo, "shared_buffer_folder", "buffers");
    MPI_Info_set(finfo, "data_buffer_size", "131072");

    MPI_File_open(MPI_COMM_WORLD, "test.out",
            MPI_MODE_CREATE | MPI_MODE_WRONLY,
            finfo, &fh);

    std::thread thread;
    if(rank == 0) {
        std::vector<std::string> dataFiles;
        std::vector<std::string> metadataFiles;
        for(int curRank = 0; curRank != size; ++curRank) {
            dataFiles.emplace_back("buffers/data-log.000" + std::to_string(curRank));
            metadataFiles.emplace_back("buffers/metadata-log.000" + std::to_string(curRank));
        }


        // int maxChunks = M_CHUNK_COUNT * 0.75f;
        int maxChunks = 0;
        int maxMasterItems = M_CHUNK_COUNT * M_ITEM_COUNT * 0.75f;


        thread = std::thread{MergeThread::StartMergeThread, "test.out", metadataFiles, dataFiles,
                                "buffers/data-log.merged",
                                maxChunks,
                                maxMasterItems,
                                16 * 1024 * 1024};
    }

    // have each one write an offset IO_SIZE number of integers
    for(int io_num = 0; io_num != IO_COUNT; ++io_num) {
        char buf[IO_SIZE];
        memset(buf, 'a' + rank, IO_SIZE);
        MPI_File_write_at_all(fh, rank * IO_SIZE + io_num * IO_SIZE * size, buf, IO_SIZE, MPI_CHAR, MPI_STATUS_IGNORE);
    }

    MPI_File_close(&fh);

    MPI_Barrier(MPI_COMM_WORLD);
    if(rank == 0) {
        MergeThread::StopMergeThread();
        thread.join();
    }

    MPI_Finalize();
    return 0;
}
