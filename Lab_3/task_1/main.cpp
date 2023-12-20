/* DON'T CHANGE THIS FILE */

#include "barrier.hpp"

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <thread>

static void random_sleep() { 
	std::this_thread::sleep_for(std::chrono::milliseconds(rand()%900+1)); 
}

int main(int argc, char **argv) {
	//init MPI environment
	MPI_Init(&argc, &argv);
	//get the rank of the actual process and the overall number of processes
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	//create the barrier communicator with size-1 processes escluding the zero-rank
	const int barrier_size = size-1;
	MPI_Comm COMM_BARRIER;
	MPI_Comm_split(MPI_COMM_WORLD, rank==0?MPI_UNDEFINED:1, rank, &COMM_BARRIER);
	//test barrier
	if(rank==0) {
		//master control thread
		int sum = 0, value;
		bool flag = false;
		const int n = 2 * barrier_size; //receives 2 messages from each process in COMM_BARRIER
		for(int i=0; i<n; ++i) {
			MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, 123, MPI_COMM_WORLD, MPI_STATUS_IGNORE);			
			if(!flag && value<0) continue; //negative values received when flag is still false are discarded
			sum += value; //sum accepted values received
			if(sum==barrier_size) flag = true; //flag becomes true when all positive values have been received
		}
		//if no message has been discarded (i.e., the barrier works) sum equals zero
		printf("%d-process barrier : test %sed.\n", barrier_size, sum==0?"pass":"fail");
	} else {
		//synchronized threads
		const int pos = +1, neg = -1;
		random_sleep();
		MPI_Ssend(&pos, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
		barrier(COMM_BARRIER);
		MPI_Ssend(&neg, 1, MPI_INT, 0, 123, MPI_COMM_WORLD);
	}
	//cleanup 
	MPI_Finalize();
	//exit
	return EXIT_SUCCESS;
}
