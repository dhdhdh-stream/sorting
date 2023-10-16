#include "branch_experiment.h"

#include "scale.h"
#include "state_network.h"

using namespace std;

const int TRAIN_EXISTING_ITERS = 100000;

void BranchExperiment::train_existing_activate(vector<ContextLayer>& context,
											   BranchExperimentHistory* history) {
	history->starting_input_state_snapshots = context.back().input_state_vals;
	history->starting_local_state_snapshots = context.back().local_state_vals;
	history->starting_score_state_snapshots = context[context.size() - this->scope_context.size()].score_state_vals;
}

void BranchExperiment::train_existing_backprop(double target_val,
											   BranchExperimentHistory* history) {
	double predicted_score = this->existing_average_score;

	for (map<int, StateStatus>::iterator it = history->starting_input_state_snapshots.begin();
			it != history->starting_input_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
		}
		map<int, Scale*>::iterator scale_it = this->existing_starting_input_state_scales.find(it->first);
		if (scale_it == this->existing_starting_input_state_scales.end()) {
			scale_it = this->existing_starting_input_state_scales.insert({it->first, new Scale(0.0)}).first;
		}
		predicted_score += scale_it->second->weight * it->second.val;
	}

	for (map<int, StateStatus>::iterator it = history->starting_local_state_snapshots.begin();
			it != history->starting_local_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
		}
		map<int, Scale*>::iterator scale_it = this->existing_starting_local_state_scales.find(it->first);
		if (scale_it == this->existing_starting_local_state_scales.end()) {
			scale_it = this->existing_starting_local_state_scales.insert({it->first, new Scale(0.0)}).first;
		}
		predicted_score += scale_it->second->weight * it->second.val;
	}

	for (map<State*, StateStatus>::iterator it = history->starting_score_state_snapshots.begin();
			it != history->starting_score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		// set for backprop in the following
		it->second.val = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;
		map<State*, Scale*>::iterator scale_it = this->existing_starting_score_state_scales.find(it->first);
		if (scale_it == this->existing_starting_score_state_scales.end()) {
			scale_it = this->existing_starting_score_state_scales.insert({it->first, new Scale(0.0)}).first;
		}
		predicted_score += scale_it->second->weight * it->second.val;
	}

	this->existing_average_score = 0.9999*this->existing_average_score + 0.0001*target_val;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->existing_average_misguess = 0.9999*this->existing_average_misguess + 0.0001*curr_misguess;

	double error = target_val - predicted_score;

	for (map<int, StateStatus>::iterator it = history->starting_input_state_snapshots.begin();
			it != history->starting_input_state_snapshots.end(); it++) {
		this->existing_starting_input_state_scales[it->first]->backprop(it->second.val*error, 0.001);
	}

	for (map<int, StateStatus>::iterator it = history->starting_local_state_snapshots.begin();
			it != history->starting_local_state_snapshots.end(); it++) {
		this->existing_starting_local_state_scales[it->first]->backprop(it->second.val*error, 0.001);
	}

	for (map<State*, StateStatus>::iterator it = history->starting_score_state_snapshots.begin();
			it != history->starting_score_state_snapshots.end(); it++) {
		this->existing_starting_score_state_scales[it->first]->backprop(it->second.val*error, 0.001);
	}
}
