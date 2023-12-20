s=1000000

run: compile
	bin/main $(s)
	
compile:
	@mkdir -p -m 700 bin && rm -rf bin/main
	g++ -O2 -Wall -fopenmp merge_sort.cpp -o bin/main

edit:
	@code -n *.cpp *makefile

clean:
	@rm -rf bin
	@chmod -x *.?pp *makefile
