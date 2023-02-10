#include "action.h"

#include "globals.h"

using namespace std;

Action::Action() {
	this->write = 0.0;
	this->move = ACTION_NONE;
}

Action::Action(double write, int move) {
	this->write = write;
	this->move = move;
}

Action::Action(ifstream& save_file) {
	string write_line;
	getline(save_file, write_line);
	this->write = stof(write_line);

	string move_line;
	getline(save_file, move_line);
	this->move = stoi(move_line);
}

Action::~Action() {
	// do nothing
}

void Action::save(ofstream& save_file) {
	save_file << this->write << endl;
	save_file << this->move << endl;
}

string Action::to_string() {
	std::string move_s;
	if (this->move == ACTION_START) {
		move_s = "START";
	} else if (this->move == ACTION_LEFT) {
		move_s = "LEFT";
	} else if (this->move == ACTION_STAY) {
		move_s = "STAY";
	} else if (this->move == ACTION_RIGHT) {
		move_s = "RIGHT";
	} else {
		move_s = "N/A";
	}
	
	return "(" + std::to_string(this->write) + "," + move_s + ")";
}
