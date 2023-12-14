#include "experiment.h"

#include <iostream>
#include <set>

#include "globals.h"
#include "run_helper.h"

using namespace std;

bool Experiment::activate(WorldState*& curr_state,
						  RunHelper& run_helper) {
	bool is_selected = false;
	if (run_helper.selected_experiment == NULL) {
		bool select = false;
		set<AbstractExperiment*>::iterator it = run_helper.experiments_seen.find(this);
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
			// leave curr_state as is
			break;
		case EXPERIMENT_STATE_TRAIN:
			train_activate(curr_state);
			break;
		case EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_state);
			break;
		case EXPERIMENT_STATE_VERIFY_EXISTING:
			// leave curr_state as is
			break;
		case EXPERIMENT_STATE_VERIFY:
			verify_activate(curr_state);
		}

		return true;
	} else {
		return false;
	}
}

void Experiment::backprop(double target_val,
						  WorldState* ending_state,
						  vector<double>& ending_state_vals) {
	switch (this->state) {
	case EXPERIMENT_STATE_MEASURE_EXISTING:
		measure_existing_backprop(target_val,
								  ending_state,
								  ending_state_vals);
		break;
	case EXPERIMENT_STATE_TRAIN:
		train_backprop(target_val,
					   ending_state,
					   ending_state_vals);
		break;
	case EXPERIMENT_STATE_MEASURE:
		measure_backprop(target_val,
						 ending_state,
						 ending_state_vals);
		break;
	case EXPERIMENT_STATE_VERIFY_EXISTING:
		verify_existing_backprop(target_val,
								 ending_state,
								 ending_state_vals);
		break;
	case EXPERIMENT_STATE_VERIFY:
		verify_backprop(target_val,
						ending_state,
						ending_state_vals);
		break;
	}
}