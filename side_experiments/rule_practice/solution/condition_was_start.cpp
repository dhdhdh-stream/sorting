#include "condition_was_start.h"

#include <iostream>

using namespace std;

ConditionWasStart::ConditionWasStart(int start_index) {
	this->type = CONDITION_TYPE_WAS_START;

	this->start_index = start_index;
}

ConditionWasStart::ConditionWasStart(ConditionWasStart* original) {
	this->type = CONDITION_TYPE_WAS_START;

	this->start_index = original->start_index;
}

ConditionWasStart::ConditionWasStart(ifstream& input_file) {
	this->type = CONDITION_TYPE_WAS_START;

	string start_index_line;
	getline(input_file, start_index_line);
	this->start_index = stoi(start_index_line);
}

bool ConditionWasStart::is_hit(vector<vector<double>>& obs_history,
							   vector<int>& move_history) {
	if (this->start_index == (int)obs_history.size()) {
		return true;
	} else {
		return false;
	}
}

void ConditionWasStart::print() {
	cout << "ConditionWasStart" << endl;
	cout << "this->start_index: " << this->start_index << endl;
}

void ConditionWasStart::save(ofstream& output_file) {
	output_file << this->start_index << endl;
}
