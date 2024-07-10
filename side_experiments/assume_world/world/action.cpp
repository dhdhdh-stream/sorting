#include "action.h"

#include <iostream>

#include "globals.h"

using namespace std;

Action::Action() {
	this->move = ACTION_NOOP;
	this->distance = 0;
}

Action::Action(int move, int distance) {
	this->move = move;
	this->distance = distance;
}

Action::Action(ifstream& save_file) {
	string move_line;
	getline(save_file, move_line);
	this->move = stoi(move_line);

	string distance_line;
	getline(save_file, distance_line);
	this->distance = stoi(distance_line);
}

Action::~Action() {
	// do nothing
}

void Action::save(ofstream& save_file) {
	save_file << this->move << endl;
	save_file << this->distance << endl;
}
