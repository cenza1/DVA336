/* DO NOT MODIFY THIS FILE */

#include "common.hpp"

// functions' semantic
#define F0(n) ((n)+16)
#define F1(n) ((n)*13)
#define G1(n) ((n)-32)
#define H1(n) ((n)*11)
#define J1(n) ((n)+64)

// functions
int f0(int n) { sleep_milliseconds(T_f0); return F0(n); }
int f1(int n) { sleep_milliseconds(T_f1); return F1(n); }
int g1(int n) { sleep_milliseconds(T_g1); return G1(n); }
int h1(int n) { sleep_milliseconds(T_h1); return H1(n); }
int j1(int n) { sleep_milliseconds(T_j1); return J1(n); }

double *wtimes = nullptr;

void in(const int dst) {
	wtimes = new double[STREAM_LENGTH];
	streamelement x;
	for(int i=0; i<STREAM_LENGTH; ++i) {
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			x[j] = f0(j);
		x.send(dst);
		wtimes[i] = -MPI_Wtime();
	}
	x.set_terminated();
	x.send(dst);
}

void out(const int src) {
	wtimes = new double[STREAM_LENGTH];
	streamelement x;
	int i = 0;
	while(true) {
		x.recv(src);
		if(x.is_terminated())
			break;
		if(i<STREAM_LENGTH)
			wtimes[i] = MPI_Wtime();
		for(int j=0; j<STREAM_ELEMENT_SIZE; ++j)
			if(x[j]!=J1(H1(G1(F1(F0(j))))))
				std::cerr << "miscalculation" << std::endl, exit(20);
		#ifdef NDEBUG
		std::cout << ++i << " " << std::flush;
		#else
		std::cout << ++i << ") " << x << std::endl;
		#endif
	}
	#ifdef NDEBUG
	std::cout << std::endl;
	#endif
	printf("stream length = %d elements processed\n", i);
}

double calc_latency() {
	if(wtimes==nullptr) {
		wtimes = new double[STREAM_LENGTH];
		for(int i=0; i<STREAM_LENGTH; ++i) wtimes[i] = 0.0;
	}
	double sums[STREAM_LENGTH], sum = 0.0;
	MPI_Allreduce(wtimes, sums, STREAM_LENGTH, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
	delete [] wtimes;
	for(int i=0; i<STREAM_LENGTH; ++i) sum += sums[i];
	return sum/STREAM_LENGTH;
}

//to be defined in the .cpp file according to the number of processes required
extern const int required_comm_size;

#undef F0
#undef F1
#undef G1
#undef H1
#undef J1