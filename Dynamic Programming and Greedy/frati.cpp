// Copyright 2018 Codreanu Bogdan

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>

using std::ifstream;
using std::ofstream;
using std::cout;
using std::cin;
using std::endl;
using std::vector;
using std::sort;
using std::abs;

#ifdef _DEBUG
#define DEBUG 1
#else
#define DEBUG 0
#endif

struct Concurs {
	int jocuri, benzi;
	// daca concursul este luat de unul dintre frati.
	bool taken;

	Concurs(int &jocuri, int &benzi) {
		this->jocuri = jocuri;
		this->benzi = benzi;
		taken = false;
	}

	void Print() {
		cout << (taken ? "[x] " : "[o] ") << jocuri << " " << benzi << endl;
	}
};

struct Game {
	// numarul de concursuri
	int n;
	// toate concursurile
	vector<Concurs*> concursuri;

	// concursurile sortate in functie de preferinta celor doi
	vector<Concurs*> concursPrefJon;
	vector<Concurs*> concursPrefSam;
	int currentIndexJon;
	int currentIndexSam;

	Game() {
		currentIndexJon = 0;
		currentIndexSam = 0;
	}

	~Game() {
		for (int i = 0; i < n; i++) {
			delete concursuri[i];
		}
	}
};

static bool CompraratorJon(const Concurs *c1, const Concurs *c2) {
	int diff = (c1->benzi + c1->jocuri) - (c2->benzi + c2->jocuri);

	if (diff == 0) {
		return c1->jocuri > c2->jocuri;
	}
	return diff > 0;
}
static bool CompraratorSam(const Concurs *c1, const Concurs *c2) {
	int diff = (c1->benzi + c1->jocuri) - (c2->benzi + c2->jocuri);

	if (diff == 0) {
		return c1->benzi > c2->benzi;
	}
	return diff > 0;
}

Game* ReadInput() {
	ifstream in;
	Game *out = new Game();
	int read1, read2;

	in.open("frati.in");

	in >> out->n;

	for (int i = 0; i < out->n; i++) {
		in >> read1 >> read2;

		// salvam concursurile citite
		out->concursuri.push_back(new Concurs(read1, read2));
		out->concursPrefJon.push_back(out->concursuri[i]);
		out->concursPrefSam.push_back(out->concursuri[i]);
	}
	// sortam in functie de preferintele fiecarui frate
	sort(out->concursPrefJon.begin(), out->concursPrefJon.end(), CompraratorJon);
	sort(out->concursPrefSam.begin(), out->concursPrefSam.end(), CompraratorSam);

	in.close();
	return out;
}

void PrintInit(Game *game) {
	cout << "********" << endl;
	cout << "Jon --- " << endl;
	for (int i = 0; i < game->n; i++) {
		game->concursPrefJon[i]->Print();
	}
	cout << "Sam --- " << endl;
	for (int i = 0; i < game->n; i++) {
		game->concursPrefSam[i]->Print();
	}
	cout << "*******" << endl;
}

void PrintAll(Game *game) {
	cout << endl << "* * * " << endl;
	for (int i = 0; i < game->n; i++) {
		cout << i << ": ";
		game->concursuri[i]->Print();
	}
	cout << "* * *" << endl;
}

// folosit pentru sari peste concursurile care au fost luate inainte de
// alt frate. de obicei while-urile vor face o singura iteratie.
// sansele de a face mai multe iteratii sunt foarte mici.
// iteratiile se intampla atunci cand:
// jon alege un concurs care e in preferinta a 5a pentru sam.
// cand sam va ajunge la a 5a alegere, va itera odata.
void RemoveTaken(Game *game) {
	while (game->concursPrefJon[game->currentIndexJon]->taken) {
		game->currentIndexJon++;
	}
	while (game->concursPrefSam[game->currentIndexSam]->taken) {
		game->currentIndexSam++;
	}
}

void SolveProblem(Game *game) {
	int puncteJon = 0;
	int puncteSam = 0;
	Concurs *currentConcurs;

	if (DEBUG)
		PrintInit(game);

	for (int i = 0; i < game->n; i++) {
		RemoveTaken(game);

		if (i % 2 == 0) {
			// miscarea lui jon

			currentConcurs = game->concursPrefJon[game->currentIndexJon];

			if (DEBUG)
				cout << endl << "* * * * JON" << endl;

			currentConcurs->taken = true;
			puncteJon += currentConcurs->jocuri;

		} else {
			// miscarea lui sam

			currentConcurs = game->concursPrefSam[game->currentIndexSam];

			if (DEBUG)
				cout << endl << "* * * * SAM" << endl;

			currentConcurs->taken = true;
			puncteSam += currentConcurs->benzi;
		}

		if (DEBUG) {
			PrintAll(game);
			cout << "scor: " << puncteJon << " " << puncteSam << endl;
		}
	}

	ofstream outFile;
	outFile.open("frati.out");
	outFile << puncteJon << " " << puncteSam;
	outFile.close();
}

int main() {
	ofstream out;
	out.open("frati.out");
	Game *game = ReadInput();
	SolveProblem(game);

	delete game;
	return 0;
}
