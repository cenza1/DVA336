/* DO NOT MODIFY THIS FILE */

#pragma once

#include "params.hpp"

#include <cassert>
#include <iostream>
#include <chrono>
#include <thread>

#define OMPI_SKIP_MPICXX 1 // needed to avoid header issues with C++-supporting MPI implementations 
#include </usr/include/openmpi-x86_64/mpi.h>

#define DIV(n,m) ((n)/(m)) //truncate the result of n/m
#define CEILDIV(n,m) DIV((n)+(m)-1,m) //roundup the result of n/m

#define TAG 1234

//define a stream element of generic size
template <int N> struct streamelement_t {
	/*
	Implementation of a stream element of generic size N.
	The entry A[N] is used to test and set the termination guard that is propagated through the modules of the graph when the stream is ended (the initial value is zero which stands for 'not terminated').
	The functions send() and recv() are used to communicate with the process passed as parameter.
	The collecting module of farm and map should be able to receive from any worker in a nondeterministic way. To this end, recv_any() receives from any process and returns the rank of the sender.
	*/
	streamelement_t() { assert(N>0); A[N] = 0; }
	bool is_terminated() const { return A[N]==1; };
	void set_terminated() { A[N] = 1; }
	int& operator [] (int i) { assert(0<=i && i<N); return A[i]; }
	void send(int dst) { MPI_Ssend(A, N+1, MPI_INT, dst, TAG, MPI_COMM_WORLD); }
	void recv(int src) { MPI_Recv(A, N+1, MPI_INT, src, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE); }
	int recv_any() { MPI_Status status; MPI_Recv(A, N+1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &status); return status.MPI_SOURCE; }
private:
	int A[N+1];
};

//overload std::cout to print a generic streamelement_t on screen
template<int N> std::ostream& operator<< (std::ostream &os, streamelement_t<N>& element) {
   os << '['; 
	for(int i=0; i<N-1; ++i) 
		os << element[i] << ','; 
	os << element[N-1] << ']';
   return os;
}

//define a stream element of a specific size
using streamelement = streamelement_t<STREAM_ELEMENT_SIZE>;

//function used int in() to init each scalar value
int f0(int n);

//functions applied to each scalar value x, i.e., j1(h1(g1(f1(x))))
int f1(int n);
int g1(int n);
int h1(int n);
int j1(int n);

//produce the stream of elements to process
void in(const int dst);

//receive the stream of results
void out(const int src);

//to be defined in the '.cpp' file according to the number of processes required to run the program
extern const int required_comm_size;

//to be implemented in the '.cpp' file
void map_rank(const int);

//blocks the execution of the current thread for at least the specified milliseconds
inline void sleep_milliseconds(const int ms) { std::this_thread::sleep_for(std::chrono::milliseconds(ms)); }

//used to measure latency by the next function
extern double *wtimes;

//measure avg latency subtracting the wall-clock time read in out() and in()
double calc_latency();
