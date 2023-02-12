#include "action.h"

#include "globals.h"

using namespace std;

Action::Action() {
	this->move = ACTION_NONE;
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

void Action::save(ofstream& save_file) {
	save_file << this->move << endl;
}

string Action::to_string() {
	std::string move_s;
	if (this->move == ACTION_START) {
		move_s = "START";
	} else if (this->move == ACTION_LEFT) {
		move_s = "LEFT";
	} else if (this->move == ACTION_RIGHT) {
		move_s = "RIGHT";
	} else if (this->move == ACTION_SWAP) {
		move_s = "SWAP";
	} else {
		move_s = "N/A";
	}
	
	return move_s;
}
