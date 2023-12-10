#include "hidden_state.h"

#include <iostream>

#include "experiment.h"

using namespace std;

HiddenState::HiddenState() {
	// do nothing
}

HiddenState::~HiddenState() {
	for (map<int, AbstractExperiment*>::iterator it = this->experiments.begin();
			it != this->experiments.end(); it++) {
		delete it->second;
	}
}

void HiddenState::activate(HiddenState*& curr_state,
						   vector<int>& action_sequence,
						   RunHelper& run_helper) {
	int curr_action = action_sequence[0];
	action_sequence.erase(action_sequence.begin());
	map<int, HiddenState*>::iterator it = this->transitions.find(curr_action);
	if (it != this->transitions.end()) {
		curr_state = it->second;
	}
	// else leave curr_state as is

	if (run_helper.type == RUN_TYPE_EXPLORE) {
		map<int, AbstractExperiment*>::iterator it = this->experiments.find(curr_action);
		if (it != this->experiments.end()) {
			it->second->activate(curr_state,
								 action_sequence,
								 run_helper);
		}
	}
}

void HiddenState::success_reset() {
	for (map<int, AbstractExperiment*>::iterator it = this->experiments.begin();
			it != this->experiments.end(); it++) {
		delete it->second;
	}
	this->experiments.clear();
}

void HiddenState::save_for_display(ofstream& output_file) {
	output_file << this->average_val << endl;

	output_file << this->transitions.size() << endl;
	for (map<int, HiddenState*>::iterator it = this->transitions.begin();
			it != this->transitions.end(); it++) {
		output_file << it->first << endl;
		output_file << it->second->id << endl;
	}
}
