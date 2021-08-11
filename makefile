main : main.cpp tokens.hpp giggaTree.hpp
	g++ main.cpp -o main -std=c++17
run : main
	./main