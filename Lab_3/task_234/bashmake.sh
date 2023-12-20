#!/bin/bash
mkdir bin 
rm -f bin/*
mpicxx -o2 -pedantic -Wall common.cpp -o bin/common.o -c
mpicxx -o2 -pedantic -Wall $1.cpp -o bin/$1.o -c
mpicxx -o2 -pedantic -Wall main.cpp bin/$1.o -o bin/main
mpicxx -o2 -pedantic -Wall getnp.cpp bin/$1.o -o bin/getnp
mpirun --oversubscribe --quiet -np `bin/getnp` bin/main
