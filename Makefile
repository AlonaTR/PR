SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.c=.hpp)
FLAGS=-DDEBUG -g

S=2
N=5
M=10
X=20

all: main

main: $(SOURCES) $(HEADERS)
	mpiCC $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main
	mpirun -n $(N) ./main $(S) $(M) $(X)
