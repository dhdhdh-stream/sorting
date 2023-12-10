#include "hmm.h"

#include <iostream>

#include "abstract_experiment.h"
#include "helpers.h"
#include "hidden_state.h"
#include "run_helper.h"

using namespace std;

HMM::HMM() {
	this->average_misguess = 0.0;
}

HMM::~HMM() {
	for (int s_index = 0; s_index < (int)this->hidden_states.size(); s_index++) {
		delete this->hidden_states[s_index];
	}
}

void HMM::init() {
	HiddenState* hidden_state = new HiddenState();
	hidden_state->average_val = 0.0;

	this->hidden_states.push_back(hidden_state);
}

void HMM::update(vector<int>& action_sequence,
				 double target_val) {
	RunHelper run_helper;
	run_helper.type = RUN_TYPE_UPDATE;

	HiddenState* curr_state = this->hidden_states[0];
	while (action_sequence.size() > 0) {
		curr_state->activate(curr_state,
							 action_sequence,
							 run_helper);
	}

	curr_state->average_val = 0.999*curr_state->average_val + 0.001*target_val;
	double curr_misguess = (target_val - curr_state->average_val) * (target_val - curr_state->average_val);
	this->average_misguess = 0.999*this->average_misguess + 0.001*curr_misguess;
}

void HMM::explore(vector<int>& action_sequence,
				  double target_val) {
	RunHelper run_helper;
	run_helper.type = RUN_TYPE_EXPLORE;

	vector<HiddenState*> state_history;
	vector<int> action_history = action_sequence;

	HiddenState* curr_state = this->hidden_states[0];
	while (action_sequence.size() > 0) {
		state_history.push_back(curr_state);
		curr_state->activate(curr_state,
							 action_sequence,
							 run_helper);
	}
	// don't need to add ending_state to history

	if (run_helper.selected_experiment != NULL) {
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
			experiment->average_remaining_experiments_from_start =
				0.9 * experiment->average_remaining_experiments_from_start
				+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index
					+ run_helper.selected_experiment->average_remaining_experiments_from_start);
		}

		run_helper.selected_experiment->backprop(target_val,
												 curr_state);

		if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_SUCCESS) {
			for (int h_index = 0; h_index < (int)this->hidden_states.size(); h_index++) {
				this->hidden_states[h_index]->success_reset();
			}
		} else if (run_helper.selected_experiment->result == EXPERIMENT_RESULT_FAIL) {
			map<int, AbstractExperiment*>::iterator it = run_helper.selected_experiment->parent->experiments.begin();
			while (true) {
				if (it->second == run_helper.selected_experiment) {
					break;
				}
				it++;
			}
			run_helper.selected_experiment->parent->experiments.erase(it);
			delete run_helper.selected_experiment;
		}
	} else {
		if (run_helper.experiments_seen.size() == 0) {
			create_experiment(state_history,
							  action_history);
		} else {
			for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
				AbstractExperiment* experiment = run_helper.experiments_seen_order[e_index];
				experiment->average_remaining_experiments_from_start =
					0.9 * experiment->average_remaining_experiments_from_start
					+ 0.1 * ((int)run_helper.experiments_seen_order.size()-1 - e_index);
			}
		}
	}
}
