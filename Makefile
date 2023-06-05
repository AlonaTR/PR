SOURCES=$(wildcard *.cpp)
HEADERS=$(SOURCES:.c=.hpp)
FLAGS=-DDEBUG -g

N=10
stanowiska=7
M=20
X=50

all: main

main: $(SOURCES) $(HEADERS)
	mpiCC $(SOURCES) $(FLAGS) -o main

clear: clean

clean:
	rm main a.out

run: main
	mpirun -n $(N) ./main $(stanowiska) $(M) $(X)
