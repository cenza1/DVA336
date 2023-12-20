/* DO NOT MODIFY THIS FILE */

#include "common.hpp"

#include <cstdio>

inline double get_wtime(MPI_Comm comm) {
	MPI_Barrier(comm);
	return MPI_Wtime();
	MPI_Barrier(comm);
}

int main(int argc, char** argv) {
	//init MPI env
	MPI_Init(&argc, &argv);
	//get process rank and communicator size
	int rank, size;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	//check that current communicator size equals the required one
	if(size!=required_comm_size)
		return EXIT_FAILURE;
	//start stream computation
	const double start = get_wtime(MPI_COMM_WORLD);
	map_rank(rank);
	const double end = get_wtime(MPI_COMM_WORLD);
	const double avg_latency = calc_latency();
	const double completion_time = end-start;
	const double avg_service_time = completion_time/STREAM_LENGTH;
	if(rank==0) {
		sleep_milliseconds(500);
		printf("number of processes = %d+2 assigned to the in() and out() functions\n"
		       "completion time = %.3f sec\n"
			    "avg service time = %.3f sec\n"
			    "avg latency = %.3f sec\n", 
			    required_comm_size-2, completion_time, avg_service_time, avg_latency);
	}
	//quit MPI env
	MPI_Finalize();
	//exit
	return EXIT_SUCCESS;
}