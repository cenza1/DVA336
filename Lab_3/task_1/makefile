np=41

run: compile
	mpirun --oversubscribe --quiet -np $(np) bin/main

compile:
	@mkdir bin && rm -f bin/*
	mpicxx -O2 -pedantic -Wall barrier.cpp -o bin/barrier.o -c
	mpicxx -O2 -pedantic -Wall main.cpp bin/barrier.o -o bin/main
	
clean:
	@rm -rf bin

install:
	sudo apt-get install libopenmpi-dev openmpi-bin
