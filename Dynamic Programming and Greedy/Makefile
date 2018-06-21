all: build

build: frati numaratoare ursi planificare

frati: frati.cpp
	g++ frati.cpp -o frati

ursi: ursi.cpp
	g++ ursi.cpp -o ursi

planificare: planificare.cpp
	g++ planificare.cpp -o planificare

numaratoare: numaratoare.cpp
	g++ numaratoare.cpp -o numaratoare

run-p1:
	./frati

run-p2:
	./ursi

run-p3:
	./planificare
	
run-p4:
	./numaratoare

clean:
	rm frati ursi planificare numaratoare