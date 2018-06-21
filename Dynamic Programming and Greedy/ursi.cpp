// Copyright 2018 Codreanu Bogdan

#include <iostream>
#include <vector>
#include <fstream>

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::ifstream;
using std::ofstream;

#define SmileEye '^'
#define SmileMouth '_'
#define MOD 1000000007

struct ReadData {
	vector<char> read;
	int eyes;
	int mouths;

	ReadData() {
		eyes = 0;
		mouths = 0;
	}
};

void SolveProblem() {
	ifstream inputFile;
	ofstream outputFile;
	ReadData readData;

	inputFile.open("ursi.in");

	// mai intai citim inputul.
	while (!inputFile.eof()) {
		char read;
		inputFile >> read;
		if (!inputFile.eof()) {
			readData.read.push_back(read);

			switch (read) {
			case SmileEye: readData.eyes++; break;
			case SmileMouth: readData.mouths++; break;
			default: break;
			}
		}
	}
	inputFile.close();

	// daca numarul de ochi cititi nu este par, atunci solutia este invalida
	if (readData.eyes % 2 != 0) {
		outputFile.open("ursi.out");
		outputFile << 0 << endl;
		outputFile.close();
		return;
	}

	// matricea de posibilitati
	int **dp;
	// nr de caractere citit (folosit ca prescurtare)
	int charsRead = readData.read.size();

	// numarul maxim pentru care este admisa deschiderea unui zambet
	// (daca am 8 ochii, inseamna pot deschide doar 4 max. dupa 4, sunt
	// obligat doar sa inchid)
	int maxSmiles = readData.eyes / 2 + 1;
	int eyesReadSoFar = 0;

	dp = new int*[charsRead];
	for (int i = 0; i < charsRead; i++) {
		dp[i] = new int[maxSmiles];

		for (int j = 0; j < maxSmiles; j++) {
			dp[i][j] = 0;
		}
	}

	// mereu vom incepe cu un zambet deschis (adica o posibilitate)
	dp[0][1] = 1;

	// prescurtare pentru dp[i][j]
	int currentFPoint;

	// pentru fiecare caracter
	for (int i = 0; i < charsRead - 1; i++) {
		// numaram al ochi este (pentru a stii daca pot deschide sau nu
		// un alt zambet)
		if (readData.read[i] == SmileEye) {
			eyesReadSoFar++;
		}

		// verific valorile "functiei" de zambete deschise
		for (int j = 0; j < maxSmiles; j++) {
			switch (readData.read[i]) {
			case SmileEye:
				currentFPoint = dp[i][j] % MOD;

				// daca sunt in punctul in care am un zambet deschis
				// si urmeaza inca un ochi, atunci inseamna ca
				// se imparte in 2 cazuri.
				// - voi mai deschide un zambet
				// - voi inchide un zambet
				// si, deci, in urmatoarea iteratie a matricei,
				// acest zambet deschis va rezulta in 2 posibilitati
				if (dp[i][j] != 0 && readData.read[i + 1] != SmileMouth) {
					// una cand deschid inca un zambet
					if (j < maxSmiles - 1) {
						dp[i + 1][j + 1] = ((dp[i + 1][j + 1] % MOD) +
							currentFPoint) % MOD;
					}

					// alta cand inchid un zambet
					if (j > 0) {
						dp[i + 1][j - 1] = ((dp[i + 1][j - 1] % MOD) +
							(1LL * currentFPoint * (j % MOD)) % MOD) % MOD;
					}
				}
				break;
			case SmileMouth:
				// daca este o gura, inseamna ca adauga
				// cate o posibilitate pentru fiecare zambet deschis
				// in momentul de fata.
				dp[i][j] = (1LL * (dp[i - 1][j] % MOD) * (j % MOD)) % MOD;
				currentFPoint = dp[i][j];

				// daca ar fi urmat un ochi, facem aceiasi logica ca mai sus
				// (este exact acelasi cod)
				if (readData.read[i + 1] != SmileMouth) {
					if (j < maxSmiles - 1) {
						dp[i + 1][j + 1] = ((dp[i + 1][j + 1] % MOD) +
							currentFPoint) % MOD;
					}

					if (j > 0) {
						dp[i + 1][j - 1] = ((dp[i + 1][j - 1] % MOD) +
							(1LL * currentFPoint * (j % MOD)) % MOD) % MOD;
					}
				}
				break;
			default: break;
			}
		}
	}

	outputFile.open("ursi.out");
	outputFile << dp[charsRead - 1][0] << endl;
	outputFile.close();

	for (int i = 0; i < charsRead; i++) {
		delete[] dp[i];
	}
	delete[] dp;
}

int main() {
	SolveProblem();
	return 0;
}
