#include "common.hpp"
#include "vector"

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */

// Function describing the behavior of the module M
void M(const int src, const int dst, int threadSize, const int rank, MPI_Comm comm_map) {
    const int elem_per_process = STREAM_ELEMENT_SIZE / threadSize;
    int totalScatterSize = elem_per_process * threadSize;

    // One thread runs the remaining elements that are left over from thee divison for elem_per_process.
    // This is done to handle uneven sizes of stream_element_size and threadSize.
    const int extra_elem_last_thread = STREAM_ELEMENT_SIZE - (totalScatterSize);

    streamelement x;
    streamelement resultElement;

    int xarr[STREAM_ELEMENT_SIZE] = {0}; // Represents the full input array of one streamelement.
    // The sub_arr is used to distrubute the needed values for all threads
    // which they will calculate on.
    int *sub_arr = nullptr;

    // Sub_res stores the calculated values of an individual thread.
    int *sub_res = nullptr;
    // The individual resultts are combined into this result array.
    int result[STREAM_ELEMENT_SIZE] = {0};

    int shouldTerminate = 0;
    int currRank;
    int comSize;
    MPI_Comm_rank(comm_map, &currRank);
    MPI_Comm_size(comm_map, &comSize);
    const int scatSize = (currRank < comSize - 1) ? elem_per_process : (elem_per_process + extra_elem_last_thread);
    sub_arr = new int[scatSize];
    sub_res = new int[scatSize];
    while (true) {
        printf("I AM RANK: %d", currRank);
        // currRank 0 will always be the rank 1 in MPI_COMM_WORLD
        // so this one will be able to send and receive from src and dst.
        // And will also make sure that every other thread is synced up to recceive
        // the next element or be terminated when needed.
        if (currRank == 0) {
            x.recv(src);
            if (x.is_terminated()) {
                shouldTerminate = 1;
                MPI_Bcast(&shouldTerminate, 1, MPI_INT, 0, comm_map);
                x.send(dst);
                return;
            }

            // Prepare next array to be shared and then broadcast that every
            // process can continue with the next scatter step. 
            for (int j = 0; j < STREAM_ELEMENT_SIZE; j++) {
                xarr[j] = x[j];
            }
            MPI_Bcast(&shouldTerminate, 1, MPI_INT, 0, comm_map);
        } else {
            MPI_Bcast(&shouldTerminate, 1, MPI_INT, 0, comm_map);
            if (shouldTerminate) {
                return;
            }
        }
        std::vector<int> sendcounts(comSize);
        std::vector<int> displs(comSize);
        for (int i = 0; i < scatSize; ++i) {
            sendcounts[i] = (i < comSize - 1) ? elem_per_process : (elem_per_process + extra_elem_last_thread);
            displs[i] = (i == 0) ? 0 : (displs[i - 1] + sendcounts[i - 1]);
    }

        // Split array of x's values into smaller sub-arrays to calculate the values in parallel.
        
        MPI_Scatterv(xarr, sendcounts.data(), displs.data(), MPI_INT, sub_arr, scatSize, MPI_INT, 0, comm_map);

        // Calculate values and store them in sub_res, each thread has it's own sub_res array which correlates with a portion of the xarr array.
        for (int j = 0; j < scatSize; j++) {
            sub_res[j] = j1(h1(g1(f1(sub_arr[j]))));
        }

        // Merge all the threads arrays into result, placing the calculated data at the correct indexes
        // in the result array.
        
        
        MPI_Gatherv(sub_res, scatSize, MPI_INT, result, sendcounts.data(), displs.data(), MPI_INT, 0, comm_map);
        
        // In the cases where STREAM_ELEMENT_SIZE is an odd number we calculate the remainder sequentially
        if (currRank == 0) {
            for (int i = 0; i < STREAM_ELEMENT_SIZE; i++) {
                x[i] = result[i];
            }

            x.send(dst);
        }
    }
}

// Set this value according to the number of processes required by the map_rank() function below
const int required_comm_size = 12;

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
    int threadSize = required_comm_size - 2;

    // Create an own MPI_Comm for the map threads so that they can use their own comm for scatter and gather.
    MPI_Comm comm_map;
    MPI_Comm_split(MPI_COMM_WORLD, (rank == 0 || rank == 2) ? MPI_UNDEFINED : 1, rank, &comm_map);

    switch (rank) {
    case 0:
        in(1);
        break;
    case 2:
        out(1);
        break;
    default:
        M(0, 2, threadSize, rank, comm_map);
        break;
    }
}
