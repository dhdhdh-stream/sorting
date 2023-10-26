/**
 * - don't worry about getting highest accuracy earliest possible
 *   - benefit to early branching might not even counter benefit to late abstraction
 *   - costs a lot of extra state/runtime
 */

#include "scope.h"

#include <iostream>
#include <queue>
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void Scope::update_histories(double target_val,
							 ScopeHistory* history) {
	if (history->exceeded_depth) {
		return;
	}

	ScopeHistory* new_scope_history = new ScopeHistory(history);
	new_scope_history->input_state_snapshots = history->input_state_snapshots;
	new_scope_history->local_state_snapshots = history->local_state_snapshots;
	if (this->scope_histories.size() >= NUM_DATAPOINTS) {
		delete this->scope_histories.front();
		this->scope_histories.pop_front();
	}
	this->scope_histories.push_back(new_scope_history);

	if (this->target_val_histories.size() >= NUM_DATAPOINTS) {
		this->target_val_histories.pop_front();
	}
	this->target_val_histories.push_back(target_val);
}

void Scope::hook() {
	for (int s_index = 0; s_index < (int)this->score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->score_state_nodes[s_index].size(); n_index++) {
			if (this->score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->score_state_nodes[s_index][n_index];

				action_node->score_state_scope_contexts.push_back(this->score_state_scope_contexts[s_index][n_index]);
				action_node->score_state_node_contexts.push_back(this->score_state_node_contexts[s_index][n_index]);
				action_node->score_state_indexes.push_back(s_index);
				action_node->score_state_defs.push_back(this->score_states[s_index]);
				action_node->score_state_network_indexes.push_back(n_index);
			} else if (this->score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->score_state_nodes[s_index][n_index];

				scope_node->score_state_scope_contexts.push_back(this->score_state_scope_contexts[s_index][n_index]);
				scope_node->score_state_node_contexts.push_back(this->score_state_node_contexts[s_index][n_index]);
				scope_node->score_state_obs_indexes.push_back(this->score_state_obs_indexes[s_index][n_index]);
				scope_node->score_state_indexes.push_back(s_index);
				scope_node->score_state_defs.push_back(this->score_states[s_index]);
				scope_node->score_state_network_indexes.push_back(n_index);
			} else {
				BranchNode* branch_node = (BranchNode*)this->score_state_nodes[s_index][n_index];

				branch_node->score_state_scope_contexts.push_back(this->score_state_scope_contexts[s_index][n_index]);
				branch_node->score_state_node_contexts.push_back(this->score_state_node_contexts[s_index][n_index]);
				branch_node->score_state_indexes.push_back(s_index);
				branch_node->score_state_defs.push_back(this->score_states[s_index]);
				branch_node->score_state_network_indexes.push_back(n_index);
			}
		}
	}
}

void Scope::score_state_helper(vector<int>& scope_context,
							   vector<int>& node_context,
							   map<int, StateStatus>& score_state_vals,
							   ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->score_state_back_activate(scope_context,
													   node_context,
													   score_state_vals,
													   action_node_history);
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				score_state_helper(scope_context,
								   node_context,
								   score_state_vals,
								   scope_node_history->inner_scope_history);

				node_context.back() = -1;

				scope_node->score_state_back_activate(scope_context,
													  node_context,
													  score_state_vals,
													  scope_node_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}



void Scope::update() {
	if (this->scope_histories.size() >= NUM_DATAPOINTS) {
		double sum_scores = 0.0;
		for (list<double>::iterator it = this->target_val_histories.begin();
				it != this->target_val_histories.end(); it++) {
			sum_scores += *it;
		}
		this->average_score = sum_scores/NUM_DATAPOINTS;
		/**
		 * - don't save averages/weights across updates
		 *   - only rely on states to preserve insights across updates
		 */

		double sum_score_variance = 0.0;
		for (list<double>::iterator it = this->target_val_histories.begin();
				it != this->target_val_histories.end(); it++) {
			sum_score_variance += (*it - this->average_score) * (*it - this->average_score);
		}
		this->score_variance = sum_score_variance/NUM_DATAPOINTS;

		Eigen::MatrixXd state_vals(NUM_DATAPOINTS, this->num_input_states + this->num_local_states + (int)this->score_states.size());
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			for (int s_index = 0; s_index < this->num_input_states + this->num_local_states + (int)this->score_states.size(); s_index++) {
				state_vals(d_index, s_index) = 0.0;
			}
		}

		hook();

		{
			int d_index = 0;
			list<ScopeHistory*>::iterator scope_it = this->scope_histories.begin();
			list<map<int, StateStatus>>::iterator input_it = this->input_state_vals_histories.begin();
			list<map<int, StateStatus>>::iterator local_it = this->local_state_vals_histories.begin();

			for (map<int, StateStatus>::iterator it = (*input_it).begin();
					it != (*input_it).end(); it++) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					state_vals(d_index, it->first) = normalized;
				} else {
					state_vals(d_index, it->first) = it->second.val;
				}
			}

			for (map<int, StateStatus>::iterator it = (*input_it).begin();
					it != (*input_it).end(); it++) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					state_vals(d_index, this->num_input_states + it->first) = normalized;
				} else {
					state_vals(d_index, this->num_input_states + it->first) = it->second.val;
				}
			}

			vector<int> scope_context;
			vector<int> node_context;
			map<int, StateStatus>& score_state_vals;
			score_state_helper(scope_context,
							   node_context,
							   score_state_vals,
							   *scope_it);

			for (map<int, StateStatus>::iterator it = score_state_vals.begin();
					it != score_state_vals.end(); it++) {
				StateNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					state_vals(d_index, this->num_input_states + this->num_local_states + it->first) = normalized;
				} else {
					state_vals(d_index, this->num_input_states + this->num_local_states + it->first) = it->second.val;
				}
			}

			d_index++;
			scope_it++;
			input_it++;
			local_it++;
		}

		unhook();

		Eigen::VectorXd target_vals(NUM_DATAPOINTS);
		{
			int d_index = 0;
			for (list<double>::iterator it = this->target_val_histories.begin();
					it != this->target_val_histories.end(); it++) {
				target_vals(d_index) = *it - this->average_score;

				d_index++;
			}
		}

		// Eigen::VectorXd weights = state_vals.fullPivHouseholderQr().solve(target_vals);
		Eigen::VectorXd weights = state_vals.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(target_vals);
		for (int s_index = 0; s_index < this->num_input_states; s_index++) {
			this->input_state_weights[s_index] = weights(s_index);
		}
		for (int s_index = 0; s_index < this->num_local_states; s_index++) {
			this->local_state_weights[s_index] = weights(this->num_input_states + s_index);
		}

		Eigen::VectorXd predicted_scores = state_vals * weights;
		Eigen::VectorXd v_diffs = target_vals - predicted_scores;
		vector<double> diffs(NUM_DATAPOINTS);
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			diffs[d_index] = v_diffs(d_index);
		}

		double sum_misguesses = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			sum_misguesses += diffs[d_index] * diffs[d_index];
		}
		this->average_misguess = sum_misguesses / NUM_DATAPOINTS;

		double sum_misguess_variance = 0.0;
		for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
			double curr_misguess = diffs[d_index] * diffs[d_index];
			sum_misguess_variance += (curr_misguess - this->average_misguess) * (curr_misguess - this->average_misguess);
		}
		this->misguess_variance = sum_misguess_variance / NUM_DATAPOINTS;

		{
			ObsExperiment* obs_experiment = create_obs_experiment(this->scope_histories.back());
			/**
			 * - simply always use most recent run
			 */
			obs_experiment->experiment(this->scope_histories,
									   diffs);
			obs_experiment->scope_eval(this);
			delete obs_experiment;
		}
	}
}
