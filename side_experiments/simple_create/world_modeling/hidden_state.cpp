#include "hidden_state.h"

#include <iostream>

#include "experiment.h"

using namespace std;

HiddenState::HiddenState() {
	this->experiment = NULL;
}

HiddenState::~HiddenState() {
	if (this->experiment != NULL) {
		delete this->experiment;
	}
}

void HiddenState::activate(HiddenState*& curr_state,
						   vector<int>& action_sequence,
						   RunHelper& run_helper) {
	if (run_helper.type == RUN_TYPE_EXPLORE && this->experiment != NULL) {
		bool is_selected = this->experiment->activate(curr_state,
													  action_sequence,
													  run_helper);
		if (is_selected) {
			return;
		}
	}

	int curr_action = action_sequence[0];
	action_sequence.erase(action_sequence.begin());
	map<int, HiddenState*>::iterator it = this->transitions.find(curr_action);
	if (it != this->transitions.end()) {
		curr_state = it->second;
	}
	// else leave curr_state as is
}
