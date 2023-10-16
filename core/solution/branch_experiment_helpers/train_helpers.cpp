#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#include <queue>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int TRAIN_PRE_ITERS = 100000;

const int OBS_EXPERIMENT_TRIES = 3;

const int TRAIN_POST_ITERS = 100000;

void BranchExperiment::train_activate(int& curr_node_id,
									  Problem& problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	history->starting_input_state_snapshots = context.back().input_state_vals;
	history->starting_local_state_snapshots = context.back().local_state_vals;
	history->starting_score_state_snapshots = context[context.size() - this->scope_context.size()].score_state_vals;
	history->starting_experiment_score_state_snapshots = context[context.size() - this->scope_context.size()].experiment_score_state_vals;

	// don't worry about curr_depth for simplicity

	// leave context.back().node_id as -1

	context.push_back(ContextLayer());

	context.back().scope_id = this->new_scope_id;

	history->sequence_histories = vector<SequenceHistory*>(this->best_step_types.size(), NULL);
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		context.back().node_id = 1 + s_index;

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->branch_experiment_train_activate(
				problem,
				context,
				run_helper);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			history->sequence_histories[s_index] = sequence_history;
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
		}

		run_helper.node_index++;

		context.back().node_id = -1;
	}

	context.pop_back();

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentHistory* history) {
	{
		double predicted_score = this->new_average_score;

		for (map<int, StateStatus>::iterator it = history->starting_input_state_snapshots.begin();
				it != history->starting_input_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				// set for backprop in the following
				it->second.val = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
			}
			map<int, Scale*>::iterator scale_it = this->new_starting_input_state_scales.find(it->first);
			if (scale_it == this->new_starting_input_state_scales.end()) {
				scale_it = this->new_starting_input_state_scales.insert({it->first, new Scale(0.0)}).first;
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
			map<int, Scale*>::iterator scale_it = this->new_starting_local_state_scales.find(it->first);
			if (scale_it == this->new_starting_local_state_scales.end()) {
				scale_it = this->new_starting_local_state_scales.insert({it->first, new Scale(0.0)}).first;
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
			map<State*, Scale*>::iterator scale_it = this->new_starting_score_state_scales.find(it->first);
			if (scale_it == this->new_starting_score_state_scales.end()) {
				scale_it = this->new_starting_score_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
			predicted_score += scale_it->second->weight * it->second.val;
		}

		for (map<State*, StateStatus>::iterator it = history->starting_experiment_score_state_snapshots.begin();
				it != history->starting_experiment_score_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			map<State*, Scale*>::iterator scale_it = this->new_starting_experiment_score_state_scales.find(it->first);
			if (scale_it == this->new_starting_experiment_score_state_scales.end()) {
				scale_it = this->new_starting_experiment_score_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
			predicted_score += scale_it->second->weight * it->second.val;
		}

		double error = target_val - predicted_score;

		for (map<int, StateStatus>::iterator it = history->starting_input_state_snapshots.begin();
				it != history->starting_input_state_snapshots.end(); it++) {
			this->new_starting_input_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<int, StateStatus>::iterator it = history->starting_local_state_snapshots.begin();
				it != history->starting_local_state_snapshots.end(); it++) {
			this->new_starting_local_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<State*, StateStatus>::iterator it = history->starting_score_state_snapshots.begin();
				it != history->starting_score_state_snapshots.end(); it++) {
			this->new_starting_score_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<State*, StateStatus>::iterator it = history->starting_experiment_score_state_snapshots.begin();
				it != history->starting_experiment_score_state_snapshots.end(); it++) {
			this->new_starting_experiment_score_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}
	}

	{
		double predicted_score = this->new_average_score;

		for (map<int, StateStatus>::iterator it = history->parent_scope_history->input_state_snapshots.begin();
				it != history->parent_scope_history->input_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				// set for backprop in the following
				it->second.val = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
			}
			map<int, Scale*>::iterator scale_it = this->new_ending_input_state_scales.find(it->first);
			if (scale_it == this->new_ending_input_state_scales.end()) {
				scale_it = this->new_ending_input_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
			predicted_score += scale_it->second->weight * it->second.val;
		}

		for (map<int, StateStatus>::iterator it = history->parent_scope_history->local_state_snapshots.begin();
				it != history->parent_scope_history->local_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				// set for backprop in the following
				it->second.val = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
			}
			map<int, Scale*>::iterator scale_it = this->new_ending_local_state_scales.find(it->first);
			if (scale_it == this->new_ending_local_state_scales.end()) {
				scale_it = this->new_ending_local_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
			predicted_score += scale_it->second->weight * it->second.val;
		}

		for (map<State*, StateStatus>::iterator it = history->parent_scope_history->score_state_snapshots.begin();
				it != history->parent_scope_history->score_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			map<State*, Scale*>::iterator scale_it = this->new_ending_score_state_scales.find(it->first);
			if (scale_it == this->new_ending_score_state_scales.end()) {
				scale_it = this->new_ending_score_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
			predicted_score += scale_it->second->weight * it->second.val;
		}

		for (map<State*, StateStatus>::iterator it = history->ending_experiment_score_state_snapshots.begin();
				it != history->ending_experiment_score_state_snapshots.end(); it++) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			// set for backprop in the following
			it->second.val = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			predicted_score += this->new_score_state_scales[it->first].first->weight * it->second.val;
		}

		this->new_average_score = 0.9999*this->new_average_score + 0.0001*target_val;
		double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
		this->new_average_misguess = 0.9999*this->new_average_misguess + 0.0001*curr_misguess;
		double curr_misguess_variance = (this->new_average_misguess - curr_misguess) * (this->new_average_misguess - curr_misguess);
		this->new_misguess_variance = 0.9999*this->new_misguess_variance + 0.0001*curr_misguess_variance;

		double error = target_val - predicted_score;

		for (map<int, StateStatus>::iterator it = history->parent_scope_history->input_state_snapshots.begin();
				it != history->parent_scope_history->input_state_snapshots.end(); it++) {
			this->new_ending_input_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<int, StateStatus>::iterator it = history->parent_scope_history->local_state_snapshots.begin();
				it != history->parent_scope_history->local_state_snapshots.end(); it++) {
			this->new_ending_local_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<State*, StateStatus>::iterator it = history->parent_scope_history->score_state_snapshots.begin();
				it != history->parent_scope_history->score_state_snapshots.end(); it++) {
			this->new_ending_score_state_scales[it->first]->backprop(it->second.val*error, 0.001);
		}

		for (map<State*, StateStatus>::iterator it = history->ending_experiment_score_state_snapshots.begin();
				it != history->ending_experiment_score_state_snapshots.end(); it++) {
			this->new_score_state_scales[it->first].first->backprop(it->second.val*error, 0.001);
		}

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
	}

	if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN_PRE) {
		this->state_iter++;
		if (this->state_iter >= TRAIN_PRE_ITERS) {
			this->state = BRANCH_EXPERIMENT_STATE_TRAIN;
			this->state_iter = 0;
		}
	} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
		/**
		 * - OK to not worry about run_helper.exceeded_depth as even if inner is cancelled, there will still be preceding obs
		 */
		if (this->obs_experiment == NULL) {
			this->state_iter++;
			if (this->state_iter <= OBS_EXPERIMENT_TRIES) {
				this->obs_experiment = create_obs_experiment(history->parent_scope_history);
			} else {
				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_POST;
				this->state_iter = 0;
			}
		}
	} else {
		// this->state == BRANCH_EXPERIMENT_STATE_TRAIN_POST
		this->state_iter++;
		if (this->state_iter >= TRAIN_POST_ITERS) {
			Scope* parent_scope = solution->scopes[this->scope_context[0]];
			double score_standard_deviation = sqrt(parent_scope->score_variance);

			// pad
			for (map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.begin();
					existing_it != this->existing_starting_input_state_scales.end(); existing_it++) {
				map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(existing_it->first);
				if (new_it == this->new_starting_input_state_scales.end()) {
					this->new_starting_input_state_scales[existing_it->first] = new Scale(0.0);
				}
			}
			for (map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.begin();
					new_it != this->new_starting_input_state_scales.end(); new_it++) {
				map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(new_it->first);
				if (existing_it == this->existing_starting_input_state_scales.end()) {
					this->existing_starting_input_state_scales[new_it->first] = new Scale(0.0);
				}
			}

			for (map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.begin();
					existing_it != this->existing_starting_local_state_scales.end(); existing_it++) {
				map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(existing_it->first);
				if (new_it == this->new_starting_local_state_scales.end()) {
					this->new_starting_local_state_scales[existing_it->first] = new Scale(0.0);
				}
			}
			for (map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.begin();
					new_it != this->new_starting_local_state_scales.end(); new_it++) {
				map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(new_it->first);
				if (existing_it == this->existing_starting_local_state_scales.end()) {
					this->existing_starting_local_state_scales[new_it->first] = new Scale(0.0);
				}
			}

			for (map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.begin();
					existing_it != this->existing_starting_score_state_scales.end(); existing_it++) {
				map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(existing_it->first);
				if (new_it == this->new_starting_score_state_scales.end()) {
					this->new_starting_score_state_scales[existing_it->first] = new Scale(0.0);
				}
			}
			for (map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.begin();
					new_it != this->new_starting_score_state_scales.end(); new_it++) {
				map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.find(new_it->first);
				if (existing_it == this->existing_starting_score_state_scales.end()) {
					this->existing_starting_score_state_scales[new_it->first] = new Scale(0.0);
				}
			}

			// remove if similar
			{
				map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.begin();
				while (existing_it != this->existing_starting_input_state_scales.end()) {
					map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(existing_it->first);
					if (abs(existing_it->second->weight - new_it->second->weight) > MIN_SCORE_IMPACT*score_standard_deviation) {
						existing_it++;
					} else {
						delete new_it->second;
						this->new_starting_input_state_scales.erase(new_it);

						delete existing_it->second;
						existing_it = this->existing_starting_input_state_scales.erase(existing_it);
					}
				}
			}

			{
				map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.begin();
				while (existing_it != this->existing_starting_local_state_scales.end()) {
					map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(existing_it->first);
					if (abs(existing_it->second->weight - new_it->second->weight) > MIN_SCORE_IMPACT*score_standard_deviation) {
						existing_it++;
					} else {
						delete new_it->second;
						this->new_starting_local_state_scales.erase(new_it);

						delete existing_it->second;
						existing_it = this->existing_starting_local_state_scales.erase(existing_it);
					}
				}
			}

			{
				map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.begin();
				while (existing_it != this->existing_starting_score_state_scales.end()) {
					// don't worry about checking if score state still exists yet
					map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(existing_it->first);
					if (abs(existing_it->second->weight - new_it->second->weight) > MIN_SCORE_IMPACT*score_standard_deviation) {
						existing_it++;
					} else {
						delete new_it->second;
						this->new_starting_score_state_scales.erase(new_it);

						delete existing_it->second;
						existing_it = this->existing_starting_score_state_scales.erase(existing_it);
					}
				}
			}

			{
				map<State*, Scale*>::iterator new_it = this->new_starting_experiment_score_state_scales.begin();
				while (new_it != this->new_starting_experiment_score_state_scales.end()) {
					if (abs(new_it->second->weight) > MIN_SCORE_IMPACT*score_standard_deviation) {
						new_it++;
					} else {
						delete new_it->second;
						new_it = this->new_starting_experiment_score_state_scales.erase(new_it);
					}
				}
			}

			this->state = BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING;
			this->state_iter = 0;
		}
	}
}
