#include "branch_experiment.h"

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

const int TRAIN_TRIES = 3;

void BranchExperiment::train_activate(int& curr_node_id,
									  Problem& problem,
									  vector<ContextLayer>& context,
									  int& exit_depth,
									  int& exit_node_id,
									  RunHelper& run_helper,
									  BranchExperimentHistory* history) {
	ScopeHistory* new_starting_scope_history = new ScopeHistory(history->parent_scope_history);
	this->new_starting_scope_histories.push_back(new_starting_scope_history);

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_input_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				(*this->new_starting_state_vals)(this->state_iter, it->first) = normalized;
			} else {
				(*this->new_starting_state_vals)(this->state_iter, it->first) = it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		if (it->first < this->containing_scope_num_local_states) {
			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				(*this->new_starting_state_vals)(this->state_iter, this->containing_scope_num_input_states + it->first) = normalized;
			} else {
				(*this->new_starting_state_vals)(this->state_iter, this->containing_scope_num_input_states + it->first) = it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			(*this->new_starting_state_vals)(this->state_iter, this->containing_scope_num_input_states + this->containing_scope_num_local_states + it->first) = normalized;
		} else {
			(*this->new_starting_state_vals)(this->state_iter, this->containing_scope_num_input_states + this->containing_scope_num_local_states + it->first) = it->second.val;
		}
	}

	history->action_histories = vector<ActionNodeHistory*>(this->best_step_types.size(), NULL);
	history->sequence_histories = vector<SequenceHistory*>(this->best_step_types.size(), NULL);
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			history->action_histories[s_index] = action_node_history;
			this->best_actions[s_index]->branch_experiment_train_activate(
				problem,
				context,
				action_node_history);
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
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::process_train() {
	double sum_scores = 0.0;
	for (int d_index = 0; d_index < this->new_target_val_histories[d_index]; d_index++) {
		sum_scores += this->new_target_val_histories[d_index];
	}
	this->new_average_score = sum_scores/NUM_DATAPOINTS;

	Eigen::VectorXd target_vals(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		target_vals(d_index) = this->new_target_val_histories[d_index] = this->new_average_score;
	}

	Eigen::VectorXd starting_weights = (*this->new_starting_state_vals).fullPivHouseholderQr().solve(target_vals);

	this->new_starting_input_state_weights = vector<double>(this->containing_scope_num_input_states);
	for (int s_index = 0; s_index < this->containing_scope_num_input_states; s_index++) {
		this->new_starting_input_state_weights[s_index] = starting_weights(s_index);
	}
	this->new_starting_local_state_weights = vector<double>(this->containing_scope_num_local_states);
	for (int s_index = 0; s_index < this->containing_scope_num_local_states; s_index++) {
		this->new_starting_local_state_weights[s_index] = starting_weights(this->containing_scope_num_input_states + s_index);
	}
	this->new_starting_experiment_state_weights = vector<double>(this->new_states.size());
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		this->new_starting_experiment_state_weights[s_index] = starting_weights(
			this->containing_scope_num_input_states + this->containing_scope_num_local_states + s_index);
	}

	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	Eigen::MatrixXd ending_state_vals(NUM_DATAPOINTS, parent_scope->num_input_states + parent_scope->num_local_states + this->new_states.size());
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		for (int s_index = 0; s_index < parent_scope->num_input_states + parent_scope->num_local_states + (int)this->new_states.size(); s_index++) {
			ending_state_vals(d_index, s_index) = 0.0;
		}
	}
	{
		int d_index = 0;
		for (list<ScopeHistory*>::iterator it = this->new_ending_scope_histories.begin();
				it != this->new_ending_scope_histories.end(); it++) {
			for (map<int, StateStatus>::iterator input_it = (*it)->input_state_snapshots.begin();
					input_it != (*it)->input_state_snapshots.end(); input_it++) {
				StateNetwork* last_network = input_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (input_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					ending_state_vals(d_index, input_it->first) = normalized;
				} else {
					ending_state_vals(d_index, input_it->first) = input_it->second.val;
				}
			}

			for (map<int, StateStatus>::iterator local_it = (*it)->local_state_snapshots.begin();
					local_it != (*it)->local_state_snapshots.end(); local_it++) {
				StateNetwork* last_network = local_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (local_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					ending_state_vals(d_index, parent_scope->num_input_states + local_it->first) = normalized;
				} else {
					ending_state_vals(d_index, parent_scope->num_input_states + local_it->first) = local_it->second.val;
				}
			}

			for (map<int, StateStatus>::iterator experiment_it = (*it)->experiment_state_snapshots.begin();
					experiment_it != (*it)->experiment_state_snapshots.end(); experiment_it++) {
				StateNetwork* last_network = experiment_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (experiment_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					ending_state_vals(d_index, parent_scope->num_input_states + parent_scope->num_local_states + experiment_it->first) = normalized;
				} else {
					ending_state_vals(d_index, parent_scope->num_input_states + parent_scope->num_local_states + experiment_it->first) = experiment_it->second.val;
				}
			}

			d_index++;
		}
	}

	Eigen::VectorXd ending_weights = ending_state_vals.fullPivHouseholderQr().solve(target_vals);

	this->new_ending_input_state_weights = vector<double>(parent_scope->num_input_states);
	for (int s_index = 0; s_index < parent_scope->num_input_states; s_index++) {
		this->new_ending_input_state_weights[s_index] = ending_weights(s_index);
	}
	this->new_ending_local_state_weights = vector<double>(parent_scope->num_local_states);
	for (int s_index = 0; s_index < parent_scope->num_local_states; s_index++) {
		this->new_ending_local_state_weights[s_index] = ending_weights(parent_scope->num_input_states + s_index);
	}
	for (int s_index = 0; s_index < (int)this->new_state_weights.size(); s_index++) {
		this->new_state_weights[s_index] = ending_weights(parent_scope->num_input_states + parent_scope->num_local_states + s_index);
	}

	Eigen::VectorXd predicted_scores = ending_state_vals * ending_weights;
	Eigen::VectorXd v_diffs = target_vals - predicted_scores;
	vector<double> diffs(NUM_DATAPOINTS);
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		diffs[d_index] = v_diffs(d_index);
	}

	double sum_misguesses = 0.0;
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		sum_misguesses += diffs[d_index] * diffs[d_index];
	}
	this->new_average_misguess = sum_misguesses / NUM_DATAPOINTS;

	double sum_misguess_variance = 0.0;
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		double curr_misguess = diffs[d_index] * diffs[d_index];
		sum_misguess_variance += (curr_misguess - this->new_average_misguess) * (curr_misguess - this->new_average_misguess);
	}
	this->new_misguess_variance = sum_misguess_variance / NUM_DATAPOINTS;

	if (this->state != BRANCH_EXPERIMENT_STATE_TRAIN_POST) {
		{
			ObsExperiment* obs_experiment = create_decision_obs_experiment(this->new_starting_scope_histories.back());
			obs_experiment->experiment(this->new_starting_scope_histories,
									   diffs);
			bool is_success = obs_experiment->branch_experiment_eval(this,
																	 true);
			if (is_success) {
				obs_experiment->update_diffs(diffs);
			}
			delete obs_experiment;
		}

		{
			ObsExperiment* obs_experiment = create_obs_experiment(this->new_ending_scope_histories.back());
			obs_experiment->experiment(this->new_ending_scope_histories,
									   diffs);
			obs_experiment->branch_experiment_eval(this,
												   false);
			delete obs_experiment;
		}
	}

	while (this->new_starting_scope_histories.size() > 0) {
		delete this->new_starting_scope_histories.front();
		this->new_starting_scope_histories.pop_front();
	}
	delete this->new_starting_state_vals;
	this->new_starting_state_vals = NULL;
	while (this->new_ending_scope_histories.size() > 0) {
		delete this->new_ending_scope_histories.front();
		this->new_ending_scope_histories.pop_front();
	}
	this->new_target_val_histories.clear();
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentHistory* history) {
	ScopeHistory* new_ending_scope_history = new ScopeHistory(history->parent_scope_history);
	new_ending_scope_history->input_state_snapshots = history->parent_scope_history->input_state_snapshots;
	new_ending_scope_history->local_state_snapshots = history->parent_scope_history->local_state_snapshots;
	new_ending_scope_history->experiment_state_snapshots = history->parent_scope_history->experiment_state_snapshots;
	this->new_ending_scope_histories.push_back(new_ending_scope_history);

	this->new_target_val_histories.push_back(target_val);

	if (this->new_target_val_histories.size() >= NUM_DATAPOINTS) {
		process_train();

		if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN_PRE) {
			Scope* containing_scope = solution->scopes[this->scope_context.back()];
			this->containing_scope_num_input_states = containing_scope->num_input_states;
			this->containing_scope_num_local_states = containing_scope->num_local_states;

			while ((int)this->existing_starting_input_state_weights.size() < this->containing_scope_num_input_states) {
				this->existing_starting_input_state_weights.push_back(0.0);
			}
			while ((int)this->existing_starting_local_state_weights.size() < this->containing_scope_num_local_states) {
				this->existing_starting_local_state_weights.push_back(0.0);
			}

			while ((int)this->new_starting_input_state_weights.size() < this->containing_scope_num_input_states) {
				this->new_starting_input_state_weights.push_back(0.0);
			}
			while ((int)this->new_starting_local_state_weights.size() < this->containing_scope_num_local_states) {
				this->new_starting_local_state_weights.push_back(0.0);
			}

			this->new_starting_state_vals = new Eigen::MatrixXd(NUM_DATAPOINTS,
				this->containing_scope_num_input_states + this->containing_scope_num_local_states + this->new_states.size());
			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
				for (int s_index = 0; s_index < this->containing_scope_num_input_states + this->containing_scope_num_local_states + (int)this->new_states.size(); s_index++) {
					(*this->new_starting_state_vals)(d_index, s_index) = 0.0;
				}
			}
			this->new_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state = BRANCH_EXPERIMENT_STATE_TRAIN;
			this->state_iter = 0;
		} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
			Scope* containing_scope = solution->scopes[this->scope_context.back()];
			this->containing_scope_num_input_states = containing_scope->num_input_states;
			this->containing_scope_num_local_states = containing_scope->num_local_states;

			while ((int)this->existing_starting_input_state_weights.size() < this->containing_scope_num_input_states) {
				this->existing_starting_input_state_weights.push_back(0.0);
			}
			while ((int)this->existing_starting_local_state_weights.size() < this->containing_scope_num_local_states) {
				this->existing_starting_local_state_weights.push_back(0.0);
			}

			while ((int)this->new_starting_input_state_weights.size() < this->containing_scope_num_input_states) {
				this->new_starting_input_state_weights.push_back(0.0);
			}
			while ((int)this->new_starting_local_state_weights.size() < this->containing_scope_num_local_states) {
				this->new_starting_local_state_weights.push_back(0.0);
			}

			this->new_starting_state_vals = new Eigen::MatrixXd(NUM_DATAPOINTS,
				this->containing_scope_num_input_states + this->containing_scope_num_local_states + this->new_states.size());
			for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
				for (int s_index = 0; s_index < this->containing_scope_num_input_states + this->containing_scope_num_local_states + (int)this->new_states.size(); s_index++) {
					(*this->new_starting_state_vals)(d_index, s_index) = 0.0;
				}
			}
			this->new_target_val_histories.reserve(NUM_DATAPOINTS);

			this->state_iter++;
			if (this->state_iter >= TRAIN_TRIES) {
				this->state = BRANCH_EXPERIMENT_STATE_TRAIN_POST;
				this->state_iter = 0;
			}
		} else {
			// BRANCH_EXPERIMENT_STATE_TRAIN_POST
			this->state = BRANCH_EXPERIMENT_STATE_MEASURE_COMBINED;
			this->state_iter = 0;
		}
	}
}
