// Copyright 2018 Codreanu Bogdan

#include <iostream>
#include <vector>
#include <fstream>
#include <algorithm>

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::ifstream;
using std::ofstream;
using std::pair;
using std::sort;


struct ProblemData {
	int s;
	int n;
	int i;

	ProblemData(int s, int n, int i) {
		this->s = s;
		this->n = n;
		this->i = i;
	}
};


ProblemData* ReadInput() {
	ifstream in;
	ProblemData *out;
	int s, n, i;
	in.open("numaratoare.in");

	in >> s >> n >> i;

	out = new ProblemData(s, n, i);

	in.close();
	return out;
}

void PrintVector(vector<int> &numbers) {
	for (int i = 0; i < numbers.size(); i++) {
		cout << numbers[i];
		if (i != numbers.size() - 1)
			cout << "+";
	}
	cout << endl;
}
void CreateNumber(int &currentIndex, int indexNeeded, int numbersNeeded,
	int sumNeeded, vector<int> &currentNumbers, int previousNumber,
	bool &foundSolution) {
	// daca am atins numarul de elemente dorite
	if (numbersNeeded == 0) {
		// si daca este indexul cerut
		if (currentIndex == indexNeeded) {
			foundSolution = true;
		}
		currentIndex++;
		return;
	}

	// daca putem sa umplem vectorul doar cu 1 pentru ca suntem obligati
	// sa mai folosim inca n numere, dar si suma este n
	if (sumNeeded == numbersNeeded) {
		// umplem direct tot vectorul de 1
		for (int i = 0; i < numbersNeeded; i++) {
			currentNumbers.push_back(1);
		}

		// apelam doar ca sa ajungem la if-ul de verificare
		CreateNumber(currentIndex, indexNeeded, 0, 0, currentNumbers, 1,
			foundSolution);

		if (foundSolution) {
			return;
		}
		for (int i = 0; i < numbersNeeded; i++) {
			currentNumbers.pop_back();
		}
		return;
	}

	// maximul posibil al unui numar este cel mai mare numar care permite
	// umplerea ulterioara a vectorului doar cu 1
	// si, bineinteles, sa nu fie mai mare decat numarul anterior trecut
	int maxNumber = std::min(sumNeeded - numbersNeeded + 1, previousNumber);
	// mergem de la maxim pana la un numar mininim (care admite posibilitatea
	// ca sumei ramase de a fi impartita in numarul de termeni ramasi)
	for (int i = maxNumber; i >= sumNeeded / numbersNeeded; i--) {
		currentNumbers.push_back(i);

		CreateNumber(currentIndex, indexNeeded, numbersNeeded - 1, sumNeeded - i,
			currentNumbers, i, foundSolution);

		if (foundSolution) {
			return;
		}

		currentNumbers.pop_back();
	}
}

void SolveProblem(ProblemData *problemData) {
	ofstream outFile;
	vector<int> numbers;

	int indexFound = 0;
	bool foundSolution = false;

	// gasim vectorul
	CreateNumber(indexFound, problemData->i, problemData->n, problemData->s,
		numbers, problemData->s, foundSolution);

	outFile.open("numaratoare.out");

	// scriem rezultatul in fisier
	if (!foundSolution) {
		outFile << "-";
	} else {
		outFile << problemData->s << "=";
		for (int i = 0; i < numbers.size(); i++) {
			outFile << numbers[i];
			if (i != numbers.size() - 1)
				outFile << "+";
		}
	}

	outFile.close();
	delete problemData;
}

int main() {
	SolveProblem(ReadInput());
	return 0;
}
