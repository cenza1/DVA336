#include "common.cpp"
#include <array>
#include <iostream>
#include <queue>

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */

// Function describing the behavior of the module M
void M(const int src, const int dst, int workerRanks[], const int workerSize) {
    streamelement x;
    streamelement result; // Used as buffer to make sure that x doesn't get overwritted by recv_any.
    std::queue<int> availableWorkers;
    int alreadyReceivedX = 0; // Set to true when an x has been received through recv_any

    for (int i = 0; i < workerSize; i++) {
        availableWorkers.push(workerRanks[i]);
    }

    while (true) {
        // Make sure that every worker has something to work on.
        // Receive a new value through x.recv(src) if a previous one haven't been received from recv_any.
        if (!availableWorkers.empty()) {
            if (alreadyReceivedX == 0) {
                x.recv(src);
            }

            if (x.is_terminated()) {
                // Make sure to receive any values that are currently being processed before
                // the termination is finalized for all workers and the dst.
                while (static_cast<int>(availableWorkers.size()) != workerSize) {
                    streamelement buff;
                    availableWorkers.push(buff.recv_any());
                    buff.send(dst);
                }

                // Terminate all workers and then terminate the dst rank
                for (int i = 0; i < workerSize; i++) {
                    x.send(workerRanks[i]);
                }

                x.send(dst);
                return;
            }

            x.send(availableWorkers.front());
            availableWorkers.pop();
            alreadyReceivedX = 0;

        } else {
            int newlyAvailableWorker = result.recv_any();

            // Handle case where an element from rank 0 is receieved from recv_any.
            // Then that one should be used next time an element is sent to a worker.
            // A worker is freed up in the process to make sure that the else-statement
            // can't be entered twice, possibly overwriting an x value that should be used.
            if (newlyAvailableWorker == src) {
                x = result;
                alreadyReceivedX = 1;
                newlyAvailableWorker = workerRanks[0];
                result.recv(newlyAvailableWorker);
            }
            availableWorkers.push(newlyAvailableWorker);
            result.send(dst);
        }
    }
}

// Recieve X from the master thread, calculate it and send it back
void Worker(const int src, const int rank) {
    streamelement x;
    while (true) {
        x.recv(src);
        if (x.is_terminated()) {
            return;
        }

        for (int i = 0; i < STREAM_ELEMENT_SIZE; i++) {
            x[i] = j1(h1(g1(f1(x[i]))));
        }
        x.send(src);
    }
}

// Set this value according to the number of processes required by the map_rank() function below
const int required_comm_size = 10;

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
    // Using the same amount of threads on Task 3 and Task 2 gives them a similiar increase in speed.
    // The difference between them is that Task 3 scales with the amount of threads executing on it.
    // This makes it a lot faster than Task 2 which threads are dependent on the pipeline stages.
    int workerRanks[7] = {3, 4, 5, 6, 7, 8, 9};
    const int workerSize = static_cast<int>(sizeof(workerRanks) / sizeof(int));

    switch (rank) {
    case 0:
        in(1);
        break;
    case 1:
        M(0, 2, workerRanks, workerSize);
        break;
    case 2:
        out(1);
        break;
    default:
        Worker(1, rank);
    }
}
