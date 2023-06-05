SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.c=.hpp)
FLAGS=-DDEBUG -g

N=5
stanowiska=2
M=10
X=25

all: main

main: $(SOURCES) $(HEADERS)
	mpiCC $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main
	mpirun -n $(N) ./main $(stanowiska) $(M) $(X)
