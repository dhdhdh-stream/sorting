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
