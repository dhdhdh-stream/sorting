#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#include <queue>

#include "action_node.h"
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

const int OBS_EXPERIMENT_TRAIN_PRE_ITERS = 100000;

const int OBS_EXPERIMENT_DECISION_TRIES = 2;
const int OBS_EXPERIMENT_FULL_TRIES = 2;

const int OBS_EXPERIMENT_TRAIN_POST_ITERS = 100000;

const double MIN_SCORE_IMPACT = 0.05;

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
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentHistory* history) {
	double predicted_score = this->average_score;

	priority_queue<pair<int, pair<bool, State*>>> backprop_queue;

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
			if (this->state_iter == 0) {
				scale_it = this->score_state_scales.insert({it->first, new Scale(it->first->scale->weight)}).first;
			} else {
				scale_it = this->score_state_scales.insert({it->first, new Scale(0.0)}).first;
			}
		}
		predicted_score += scale_it->second->weight * it->second.val;

		backprop_queue.push({it->second.last_updated, {true, it->first}});
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

		backprop_queue.push({it->second.last_updated, {false, it->first}});
	}

	if (this->obs_experiment != NULL) {
		backprop_queue.push({history->parent_scope_history->test_last_updated+1, {false, NULL}});
		/**
		 * - increment by 1 so that it triggers after existing
		 */
	}

	this->average_score = 0.9999*this->average_score + 0.0001*target_val;
	double curr_misguess = (target_val - predicted_score) * (target_val - predicted_score);
	this->average_misguess = 0.9999*this->average_misguess + 0.0001*curr_misguess;
	double curr_misguess_variance = (this->average_misguess - curr_misguess) * (this->average_misguess - curr_misguess);
	this->misguess_variance = 0.9999*this->misguess_variance + 0.0001*curr_misguess_variance;

	while (!backprop_queue.empty()) {
		if (backprop_queue.top().second.second == NULL) {
			this->obs_experiment->backprop(target_val,
										   predicted_score,
										   history->parent_scope_history->test_obs_indexes,
										   history->parent_scope_history->test_obs_vals);

			if (this->obs_experiment->state == OBS_EXPERIMENT_STATE_DONE) {
				this->obs_experiment->branch_experiment_eval(this);

				delete this->obs_experiment;
				this->obs_experiment = NULL;
			}
		} else {
			double error = target_val - predicted_score;

			if (backprop_queue.top().second.first) {
				map<State*, StateStatus>::iterator it = history->parent_scope_history->score_state_snapshots.find(backprop_queue.top().second.second);

				map<State*, Scale*>::iterator scale_it = this->score_state_scales.find(it->first);
				scale_it->second->backprop(it->second.val*error, 0.001);

				predicted_score += scale_it->second->weight * it->second.val;
			} else {
				map<State*, StateStatus>::iterator it = history->experiment_score_state_snapshots.find(backprop_queue.top().second.second);

				it->first->scale->backprop(it->second.val*error, 0.001);

				// don't worry about removing score states for now

				predicted_score -= it->first->scale->weight * it->second.val;
			}
		}

		backprop_queue.pop();
	}

	if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN_PRE) {
		this->state_iter++;
		if (this->state_iter >= OBS_EXPERIMENT_TRAIN_PRE_ITERS) {
			this->state = BRANCH_EXPERIMENT_STATE_TRAIN;
			this->state_iter = 0;
		}
	} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
		/**
		 * - OK to not worry about run_helper.exceeded_depth as even if inner is cancelled, there will still be preceding obs
		 */
		if (this->obs_experiment == NULL) {
			this->state_iter++;
			if (this->state_iter <= OBS_EXPERIMENT_DECISION_TRIES) {
				this->obs_experiment = create_decision_obs_experiment(history->parent_scope_history);
			} else if (this->state_iter <= OBS_EXPERIMENT_DECISION_TRIES + OBS_EXPERIMENT_FULL_TRIES) {
				this->obs_experiment = create_full_obs_experiment(history->parent_scope_history);
			} else {
				Scope* parent_scope = solution->scopes[this->scope_context[0]];
				double score_standard_deviation = sqrt(parent_scope->score_variance);
				map<State*, Scale*>::iterator it = this->score_state_scales.begin();
				while (it != this->score_state_scales.end()) {
					double original_impact = it->first->resolved_standard_deviation * it->first->scale->weight;
					double new_impact = it->first->resolved_standard_deviation * it->second->weight;
					if (it->first->nodes.size() != 0
							&& abs(original_impact - new_impact) > MIN_SCORE_IMPACT*score_standard_deviation) {
						it++;
					} else {
						it = this->score_state_scales.erase(it);
					}
				}

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_POST;
				this->state_iter = 0;
			}
		}
	} else {
		// this->state == BRANCH_EXPERIMENT_STATE_TRAIN_POST
		this->state_iter++;
		if (this->state_iter >= OBS_EXPERIMENT_TRAIN_POST_ITERS) {
			this->state = BRANCH_EXPERIMENT_STATE_MEASURE_EXISTING;
			this->state_iter = 0;
		}
	}
}
