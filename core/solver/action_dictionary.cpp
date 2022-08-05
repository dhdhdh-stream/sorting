#include "action_dictionary.h"

#include <cmath>
#include <iostream>

using namespace std;

ActionDictionary::ActionDictionary() {
	// do nothing
}

ActionDictionary::ActionDictionary(std::ifstream& save_file) {
	string num_actions_line;
	getline(save_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		CompoundAction* action = new CompoundAction(save_file);
		this->actions.push_back(action);

		string num_success_line;
		getline(save_file, num_success_line);
		this->num_success.push_back(stoi(num_success_line));

		string count_line;
		getline(save_file, count_line);
		this->count.push_back(stoi(count_line));
	}

	string total_count_line;
	getline(save_file, total_count_line);
	this->total_count = stoi(total_count_line);
}

ActionDictionary::~ActionDictionary() {
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		delete this->actions[a_index];
	}
}

int ActionDictionary::select_compound_action() {
	// TODO: don't use UCT as want random combinations

	this->total_count++;

	this->mtx.lock();
	int num_actions = (int)this->actions.size();
	this->mtx.unlock();

	double best_uct = numeric_limits<double>::lowest();
	int best_index = -1;
	for (int a_index = 0; a_index < num_actions; a_index++) {
		double uct = (double)this->num_success[a_index]/(double)(this->count[a_index]+1) \
			+ 1.414*sqrt(log(this->total_count+1)/(this->count[a_index]+1));

		if (uct > best_uct) {
			best_uct = uct;
			best_index = a_index;
		}
	}

	return best_index;
}

void ActionDictionary::add_action(vector<Action> action_sequence) {
	CompoundAction* new_action = new CompoundAction(action_sequence);
	
	this->mtx.lock();
	this->actions.push_back(new_action);
	this->num_success.push_back(0);
	this->count.push_back(0);
	this->mtx.unlock();
}

int ActionDictionary::calculate_action_path_length(Action a) {
	if (a.move != COMPOUND) {
		return 1;
	}

	int sum_length = 0;

	CompoundAction* compound_action = this->actions[a.compound_index];

	CompoundActionNode* curr_node = compound_action->nodes[1];
	while (true) {
		if (curr_node->children_indexes[0] == 0) {
			break;
		}

		Action a = curr_node->children_actions[0];
		int a_length = calculate_action_path_length(a);
		sum_length += a_length;

		curr_node = compound_action->nodes[curr_node->children_indexes[0]];
	}

	return sum_length;
}

void ActionDictionary::convert_to_raw_actions(Action a,
											  vector<Action>& raw_actions) {
	if (a.move != COMPOUND) {
		raw_actions.push_back(a);
		return;
	}

	CompoundAction* compound_action = this->actions[a.compound_index];

	CompoundActionNode* curr_node = compound_action->nodes[1];
	while (true) {
		if (curr_node->children_indexes[0] == 0) {
			break;
		}

		Action a = curr_node->children_actions[0];
		convert_to_raw_actions(a, raw_actions);

		curr_node = compound_action->nodes[curr_node->children_indexes[0]];
	}
}

void ActionDictionary::save(ofstream& save_file) {
	this->mtx.lock();
	int num_actions = (int)this->actions.size();
	this->mtx.unlock();

	save_file << num_actions << endl;
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->actions[a_index]->save(save_file);

		save_file << this->num_success[a_index] << endl;
		save_file << this->count[a_index] << endl;
	}
	save_file << this->total_count << endl;
}
