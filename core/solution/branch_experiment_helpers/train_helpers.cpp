#include "branch_experiment.h"

#include "action_node.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int OBS_EXPERIMENT_TRIES = 6;

void BranchExperiment::train_activate(int& curr_node_id,
									  Problem& problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	history->sequence_histories = vector<SequenceHistory*>(this->best_step_types.size(), NULL);
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->branch_experiment_train_activate(
				problem,
				context);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			history->sequence_histories[s_index] = sequence_history;
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentHistory* history) {
	double predicted_score = this->average_score;

	for (map<State*, StateStatus>::iterator it = history->parent_scope_history->score_state_snapshots.begin();
			it != history->parent_scope_history->score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
		if (scale_it == this->score_state_scales.end()) {
			scale_it = this->score_state_scales.insert({it->first, new Scale(it->first->scale->weight)}).first;
		}
		predicted_score += scale_it->second->weight * it->second.val;
	}

	for (map<State*, StateStatus>::iterator it = history->experiment_score_state_snapshots.begin();
			it != history->experiment_score_state_snapshots.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		if (it->first->resolved_network_indexes.find(last_network->index) == it->first->resolved_network_indexes.end()) {
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation * last_network->correlation_to_end
				* it->first->resolved_standard_deviation;
		}
		predicted_score += it->first->scale->weight * it->second.val;
	}

	double error = target_val - predicted_score;

	for (map<State*, StateStatus>::iterator it = history->parent_scope_history->score_state_snapshots.begin();
			it != history->parent_scope_history->score_state_snapshots.end(); it++) {
		map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
		scale_it->second->backprop(it->second.val*error, 0.001);
	}

	for (map<State*, StateStatus>::iterator it = history->experiment_score_state_snapshots.begin();
			it != history->experiment_score_state_snapshots.end(); it++) {
		it->first->scale->backprop(it->second.val*error, 0.001);

		// don't worry about removing score states for now
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	if (this->obs_experiment != NULL) {
		this->obs_experiment->backprop(target_val,
									   predicted_score,
									   history->parent_scope_history->test_obs_indexes,
									   history->parent_scope_history->test_obs_vals);

		if (this->obs_experiment->state == OBS_EXPERIMENT_STATE_DONE) {
			this->obs_experiment->branch_experiment_eval(this);

			delete this->obs_experiment;
			this->obs_experiment = NULL;
		}
	}

	if (this->obs_experiment == NULL) {
		this->state_iter++;
		if (this->state_iter <= OBS_EXPERIMENT_TRIES) {
			this->obs_experiment = create_obs_experiment(history->parent_scope_history);
		} else {
			this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
