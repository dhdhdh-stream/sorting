#include "action.h"

using namespace std;

Action::Action(double write, int move) {
	this->write = write;
	this->move = move;
	this->index = -1;
}

Action::Action(int move, int index) {
	this->write = 0.0;
	this->move = move;
	this->index = index;
}

Action::Action(ifstream& save_file) {
	string write_line;
	getline(save_file, write_line);
	this->write = stof(write_line);

	string move_line;
	getline(save_file, move_line);
	this->move = stoi(move_line);

	string index_line;
	getline(save_file, index_line);
	this->index = stoi(index_line);
}

Action::~Action() {
	// do nothing
}

void Action::save(ofstream& save_file) {
	save_file << this->write << endl;
	save_file << this->move << endl;
	save_file << this->index << endl;
}

string Action::to_string() {
	if (this->move == COMPOUND) {
		return "C " + std::to_string(this->index);
	} else if (this->move == LOOP) {
		return "L " + std::to_string(this->index);
	} else {
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
}
