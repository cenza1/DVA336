cpps=$(filter-out common.cpp getnp.cpp main.cpp,$(wildcard *.cpp))

$(cpps):
	@mkdir -p -m 700 bin && rm -f bin/*
	mpicxx -O2 -pedantic -Wall $@ -o bin/$@.o -c
	mpicxx -O2 -pedantic -Wall common.cpp -o bin/common.o -c
	mpicxx -O2 -pedantic -Wall main.cpp bin/*.o -o bin/main
	mpicxx -O2 -pedantic -Wall getnp.cpp bin/*.o -o bin/getnp
	mpirun --oversubscribe --quiet -np `bin/getnp` bin/main

clean:
	@rm -rf bin

install:
	sudo apt-get install libopenmpi-dev openmpi-bin
