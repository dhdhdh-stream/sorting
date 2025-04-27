#include "condition_action_equals.h"

#include <iostream>

using namespace std;

ConditionActionEquals::ConditionActionEquals(int action_index,
											 int move) {
	this->type = CONDITION_TYPE_ACTION_EQUALS;

	this->action_index = action_index;
	this->move = move;
}

ConditionActionEquals::ConditionActionEquals(ConditionActionEquals* original) {
	this->type = CONDITION_TYPE_ACTION_EQUALS;

	this->action_index = original->action_index;
	this->move = original->move;
}

ConditionActionEquals::ConditionActionEquals(ifstream& input_file) {
	this->type = CONDITION_TYPE_ACTION_EQUALS;

	string action_index_line;
	getline(input_file, action_index_line);
	this->action_index = stoi(action_index_line);

	string move_line;
	getline(input_file, move_line);
	this->move = stoi(move_line);
}

bool ConditionActionEquals::is_hit(vector<vector<double>>& obs_history,
								   vector<int>& move_history) {
	if (this->action_index > (int)move_history.size()) {
		return false;
	} else {
		if (move_history[move_history.size() - this->action_index] >= this->move) {
			return true;
		} else {
			return false;
		}
	}
}

void ConditionActionEquals::print() {
	cout << "ConditionActionEquals" << endl;
	cout << "this->action_index: " << this->action_index << endl;
	cout << "this->move: " << this->move << endl;
}

void ConditionActionEquals::save(ofstream& output_file) {
	output_file << this->action_index << endl;
	output_file << this->move << endl;
}
