#include "scope.h"

#include <iostream>
#include <queue>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

/**
 * - don't worry about getting highest accuracy earliest possible
 *   - benefit to early branching might not even counter benefit to late abstraction
 *   - costs a lot of extra state/runtime
 */
void Scope::update_backprop(double target_val,
							RunHelper& run_helper,
							ScopeHistory* history,
							set<pair<State*, Scope*>>& states_to_remove) {
	if (history->exceeded_depth) {
		return;
	}

	double predicted_score = this->average_score;
	
	for (map<int, StateStatus>::iterator it = history->input_state_snapshots.begin();
			it != history->input_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
		}
		predicted_score += this->input_state_scales[it->first]->weight * it->second.val;
	}

	for (map<int, StateStatus>::iterator it = history->local_state_snapshots.begin();
			it != history->local_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
		}
		predicted_score += this->local_state_scales[it->first]->weight * it->second.val;
	}

	for (map<State*, StateStatus>::iterator it = history->score_state_snapshots.begin();
			it != history->score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		// set for backprop in the following
		it->second.val = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;
		predicted_score += this->score_state_scales[it->first].first->weight * it->second.val;
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_score_variance = (this->average_score - target_val) * (this->average_score - target_val);
	this->score_variance = 0.9999*this->score_variance + 0.0001*curr_score_variance;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	double error = target_val - predicted_score;

	for (map<int, StateStatus>::iterator it = history->input_state_snapshots.begin();
			it != history->input_state_snapshots.end(); it++) {
		this->input_state_scales[it->first]->backprop(it->second.val*error, 0.001);
	}

	for (map<int, StateStatus>::iterator it = history->local_state_snapshots.begin();
			it != history->local_state_snapshots.end(); it++) {
		this->local_state_scales[it->first]->backprop(it->second.val*error, 0.001);
	}

	for (map<State*, StateStatus>::iterator it = history->score_state_snapshots.begin();
			it != history->score_state_snapshots.end(); it++) {
		map<State*, std::pair<Scale*, double>>::iterator scale_it = this->score_state_scales.find(it->first);
		scale_it->second.first->backprop(it->second.val*error, 0.001);

		if (scale_it->second.first->weight < 0.05*scale_it->second.second) {
			states_to_remove.insert({it->first, this});
		}
	}

	if (this->obs_experiment != NULL) {
		this->obs_experiment->backprop(target_val,
									   predicted_score,
									   history->test_obs_indexes,
									   history->test_obs_vals);

		if (this->obs_experiment->state == OBS_EXPERIMENT_STATE_DONE) {
			this->obs_experiment->scope_eval(this);

			delete this->obs_experiment;
			this->obs_experiment = NULL;
		}
	}

	if (this->obs_experiment == NULL && !run_helper.exceeded_depth) {
		this->obs_experiment = create_obs_experiment(history);
	}
}
