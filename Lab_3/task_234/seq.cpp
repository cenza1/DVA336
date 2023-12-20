#include "common.hpp"

/* Here is the list of functions describing the behaviour of each module, except in() and out() defined in common.cpp */

// Function describing the behavior of the module M
void M(const int src, const int dst) {
	streamelement x;
	while(true) {
		x.recv(src);
		if(x.is_terminated()) {
			x.send(dst);
			return;
		}
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = j1(h1(g1(f1(x[j]))));
		x.send(dst);
	}
}

// Set this value according to the number of processes required by the map_rank() function below
const int required_comm_size = 3; 

// This function maps each module to a distinct rank associated to a process (mind that each process is associated with exactly one module)
void map_rank(const int rank) {
	if(rank==0) in(1);
	if(rank==1) M(0,2);
	if(rank==2) out(1);
}
