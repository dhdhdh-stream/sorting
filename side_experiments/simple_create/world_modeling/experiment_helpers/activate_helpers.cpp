#include "experiment.h"

#include <iostream>
#include <set>

#include "globals.h"
#include "run_helper.h"

using namespace std;

bool Experiment::activate(HiddenState*& curr_state,
						  vector<int>& action_sequence,
						  RunHelper& run_helper) {
	if (action_sequence[0] != this->starting_action) {
		return false;
	}

	bool is_selected = false;
	if (run_helper.selected_experiment == NULL) {
		bool select = false;
		set<Experiment*>::iterator it = run_helper.experiments_seen.find(this);
		if (it == run_helper.experiments_seen.end()) {
			double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < selected_probability) {
				select = true;
			}

			run_helper.experiments_seen_order.push_back(this);
			run_helper.experiments_seen.insert(this);
		}
		if (select) {
			run_helper.selected_experiment = this;

			is_selected = true;
		}
	} else if (run_helper.selected_experiment == this) {
		is_selected = true;
	}

	if (is_selected) {
		switch (this->state) {
		case EXPERIMENT_STATE_MEASURE_EXISTING:
			measure_existing_activate(curr_state,
									  action_sequence);
			break;
		case EXPERIMENT_STATE_TRAIN:
			train_activate(curr_state,
						   action_sequence);
			break;
		case EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_state,
							 action_sequence);
			break;
		case EXPERIMENT_STATE_VERIFY_EXISTING:
			verify_existing_activate(curr_state,
									 action_sequence);
			break;
		case EXPERIMENT_STATE_VERIFY:
			verify_activate(curr_state,
							action_sequence);
		}

		return true;
	} else {
		return false;
	}
}

void Experiment::backprop(double target_val,
						  HiddenState* ending_state) {
	switch (this->state) {
	case EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  ending_state);
		break;
	case EXPERIMENT_STATE_TRAIN:
		train_backprop(target_val,
					   ending_state);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 ending_state);
		break;
	case EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 ending_state);
		break;
	case EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val,
						ending_state);
		break;
	}
}
