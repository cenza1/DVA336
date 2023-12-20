#include "barrier.hpp"
#include <cstdio>

// The solution will have a total related depth of 3 as the whole program is
// finished when the original master control thread is done,
// which is dependent on the barrier function to finish.
// The barrier function has its own control thread which requires all processes
// to have communicated with the control thread in order for the function to finish.

void barrier(MPI_Comm COMM_BARRIER) {
    /* Your solution goes here (mind to use COMM_BARRIER as communicator instead of MPI_COMM_WORLD) */
    int totalSize, currRank;
    MPI_Comm_size(COMM_BARRIER, &totalSize);
    MPI_Comm_rank(COMM_BARRIER, &currRank);
    // Thread with rank 0 becomes control thread
    if (currRank == 0) {
        int sum = 0, value = 0;
        // Synchronize all threads
        for (int i = 0; i < totalSize - 1; i++) {
            MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, 123, COMM_BARRIER, MPI_STATUS_IGNORE);
            sum += value;
        }
        // Broadcast to all threads when they are synched
        if (sum == (totalSize - 1)) {
            MPI_Bcast(&sum, 1, MPI_INT, 0, COMM_BARRIER);
        }
    } else {
        int buf = 1;
        // Send a message to control thread and wait to be synched
        MPI_Ssend(&buf, 1, MPI_INT, 0, 123, COMM_BARRIER);
        // Recieve broadcast and return to main
        MPI_Bcast(&buf, 1, MPI_INT, 0, COMM_BARRIER);
    }
}
