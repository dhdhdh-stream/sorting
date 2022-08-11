#include "action_dictionary.h"

#include <cmath>
#include <iostream>

#include "definitions.h"
#include "loop.h"

using namespace std;

ActionDictionary::ActionDictionary() {
	this->total_count = 0;
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