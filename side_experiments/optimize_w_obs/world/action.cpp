#include "action.h"

#include <iostream>

#include "globals.h"

using namespace std;

Action::Action() {
	this->move = ACTION_NOOP;
}

Action::Action(int move) {
	this->move = move;
}

Action::Action(ifstream& save_file) {
	string move_line;
	getline(save_file, move_line);
	this->move = stoi(move_line);
}

Action::~Action() {
	// do nothing
}

bool Action::operator==(const Action& rhs) {
	return this->move == rhs.move;
}

void Action::save(ofstream& save_file) {
	save_file << this->move << endl;
}
