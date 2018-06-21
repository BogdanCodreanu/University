// Copyright 2018 Codreanu Bogdan

#include <math.h>
#include <iostream>
#include <vector>
#include <fstream>

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::ifstream;
using std::ofstream;

int Pow3(int nr) {
	return pow(nr, 3);
}

struct Proba {
	int durata;
	// pierderea minima pana la concursul asta
	long long int minimalLossSoFar;
	// numarul de concursuri minime folosite
	int nrConcursuriFolosite;


	explicit Proba(int durata) {
		this->durata = durata;
	}
};

void SolveProblem() {
	ifstream inputFile;
	ofstream outputFile;
	int breakTime, nrProbe, durataConcurs;
	vector<Proba> probe;

	inputFile.open("planificare.in");
	inputFile >> nrProbe >> durataConcurs >> breakTime;

	for (int i = 0; i < nrProbe; i++) {
		int read;
		inputFile >> read;
		probe.push_back(Proba(read));
	}
	inputFile.close();

	long long int currentFreeTime;
	int j;
	// pentru fiecare proba
	for (int i = 0; i < nrProbe; i++) {
		// mai intai calculam un timp liber (in care se vor putea baga
		// alte concursuri)
		currentFreeTime = durataConcurs - probe[i].durata;

		if (i == nrProbe - 1) {
			// daca ar fi ultima proba, atunci nu mai exista pierdere de la proba
			// pana la finalul unui eventual concurs
			probe[i].minimalLossSoFar = probe[i - 1].minimalLossSoFar;
			probe[i].nrConcursuriFolosite = probe[i - 1].nrConcursuriFolosite + 1;

		} else if (i == 0) {
			// daca este prima proba, atunci nu exista un concurs anterior
			// de la care sa preia date. (acesta este cazul de baza)
			probe[i].minimalLossSoFar = Pow3(currentFreeTime);
			probe[i].nrConcursuriFolosite = 1;
		} else {
			// pierderea minima, de obicei, este timpul liber ridicat la a 3a
			// plus ce pierderi am avut inainte de asta,
			probe[i].minimalLossSoFar = Pow3(currentFreeTime) +
				probe[i - 1].minimalLossSoFar;
			// presupunand ca este un nou concurs
			probe[i].nrConcursuriFolosite = probe[i - 1].nrConcursuriFolosite + 1;
		}

		// acum incepem sa vereificam ce probe anterioare pot fi bagate in
		// acest concurs
		j = i - 1;
		while (j >= 0 && currentFreeTime - (probe[j].durata + breakTime)
			>= 0) {
			// incercam sa bagam o proba in acest concurs.
			currentFreeTime -= (probe[j].durata + breakTime);

			if (j != 0) {
				// daca, ipotetic, am baga acesta proba, inseamna ca am avea
				// pierderea minima = Pow3(currentFreeTime) + ce era anterior
				// de aceasta noua proba bagata.
				if (Pow3(currentFreeTime) + probe[j - 1].minimalLossSoFar <
					probe[i].minimalLossSoFar) {
					// daca e un rezultat mai bun, atunci chiar o bagam
					// in concursul actual
					probe[i].minimalLossSoFar = Pow3(currentFreeTime) +
						probe[j - 1].minimalLossSoFar;
					// iar numarul de concursuri totale devine
					// numarul de concursuri probei anterioare celei nou
					// bagate + 1. pentru ca avem doar un concurs in plus,
					// cel de acum.
					probe[i].nrConcursuriFolosite =
						probe[j - 1].nrConcursuriFolosite + 1;
				}
			} else {
				// cazul de baza, cand j == 0
				if (Pow3(currentFreeTime) < probe[i].minimalLossSoFar) {
					probe[i].minimalLossSoFar = Pow3(currentFreeTime);
					probe[i].nrConcursuriFolosite = 1;
				}
			}
			j--;
		}
	}

	outputFile.open("planificare.out");
	outputFile << probe[nrProbe - 1].minimalLossSoFar << " " <<
		probe[nrProbe - 1].nrConcursuriFolosite;
	outputFile.close();
}

int main() {
	SolveProblem();
	return 0;
}
