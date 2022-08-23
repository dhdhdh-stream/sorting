#include "action.h"

using namespace std;

Action::Action() {
	this->write = 0.0;
	this->move = -1;
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
	if (this->move == LEFT) {
		move_s = "LEFT";
	} else if (this->move == STAY) {
		move_s = "STAY";
	} else {
		move_s = "RIGHT";
	}
	
	return "(" + std::to_string(this->write) + "," + move_s + ")";
}
