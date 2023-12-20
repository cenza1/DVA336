#include "common.hpp"

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */

// Function describing the behavior of the module M
void M(const int src, const int dst, const int rank, const int dstRank) {

    streamelement x;
    while (true) {
        x.recv(src);
        if (x.is_terminated()) {
            x.send(dst);
            return;
        }
        //Send all elements in bulk to the first stage
        int xarr[STREAM_ELEMENT_SIZE] = { 0 };
        for (int j = 0; j < STREAM_ELEMENT_SIZE; j++) {
            xarr[j] = x[j];
        }
        MPI_Ssend(&xarr, STREAM_ELEMENT_SIZE, MPI_INT, dstRank, 100, MPI_COMM_WORLD);

        //Recieve results coming out of the pipeline
        for (int j = 0; j < STREAM_ELEMENT_SIZE; j++) {
            int result = -1;
            MPI_Recv(&result, 1, MPI_INT, MPI_ANY_SOURCE, j, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            x[j] = result;
        }

        x.send(dst);
    }
}


void calculateStage1(const int rank, const int destRank) {
    int receivedArr[STREAM_ELEMENT_SIZE] = { 0 };
    int received = 0;

    // Receives the elements as a bulk in an array and then starts working on them separetly,
    // sending every processed element to the next step in the pipeline and then starts the next calculation.
    for (int i = 0; i < STREAM_LENGTH; i++) {
        MPI_Recv(&receivedArr, STREAM_ELEMENT_SIZE, MPI_INT, MPI_ANY_SOURCE, 100, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Fully calculates all the elements in a stream element before moving on to receive data from the next one
        for (int j = 0; j < STREAM_ELEMENT_SIZE; j++) {
          received = f1(receivedArr[j]);
          MPI_Ssend(&received, 1, MPI_INT, destRank, j, MPI_COMM_WORLD);
        }
    }
}

//Recieve result from stage 1, calculate result of h1 and g1, then send it to stage 3
// Tag becomes unique identifier for every element in the stream,
// element can then be recievedvcorrectly based on the tag.
void calculateStage2(const int rank, const int destRank) {
    int received = -1;
    for (int i = 0, tag = 0; i < STREAM_LENGTH * STREAM_ELEMENT_SIZE; i++, tag = i % STREAM_ELEMENT_SIZE) {
        MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Calculates both h1 and g1 since they have short service times, making the needed time per stage more even.
        // The total service time of h1+g1 will be 13 which is still shorter than the service times of f1 or j1.
        received = h1(g1(received));
        MPI_Ssend(&received, 1, MPI_INT, destRank, tag, MPI_COMM_WORLD);
    }
}

//Recieve result from stage 2, calculate the final result with j1 and send it back to M.
void calculateStage3(const int rank, const int destRank) {
    int received = -1;
    for (int i = 0, tag = 0; i < STREAM_LENGTH * STREAM_ELEMENT_SIZE; i++, tag = i % STREAM_ELEMENT_SIZE) {
        MPI_Recv(&received, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        received = j1(received);
        MPI_Ssend(&received, 1, MPI_INT, destRank, tag, MPI_COMM_WORLD);
    }
}

// Set this value according to the number of processes required by the map_rank() function below
const int required_comm_size = 6;

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
    switch (rank) {
    case 0:
        in(1);
        break;
    case 1:
        M(0, 2, rank, 3);
        break;
    case 2:
        out(1);
        break;
    case 3:
        calculateStage1(rank, 4);
        break;
    case 4:
        calculateStage2(rank, 5);
        break;
    case 5:
        calculateStage3(rank, 1);
        break;
    }
}
