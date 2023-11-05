#include "branch_experiment.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "state_network.h"

using namespace std;

void BranchExperiment::train_existing_activate(vector<ContextLayer>& context) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	vector<map<int, StateStatus>> input_state_vals_snapshot(this->scope_context.size());
	vector<map<int, StateStatus>> local_state_vals_snapshot(this->scope_context.size());
	vector<map<State*, StateStatus>> temp_state_vals_snapshot(this->scope_context.size());
	for (int c_index = 0; c_index < this->scope_context.size(); c_index++) {
		input_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].input_state_vals;
		local_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].local_state_vals;
		temp_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].temp_state_vals;
	}
	this->i_input_state_vals_histories.push_back(context.back().input_state_vals);
	this->i_local_state_vals_histories.push_back(context.back().local_state_vals);
	this->i_temp_state_vals_histories.push_back(context.back().temp_state_vals);

	BranchExperimentOverallHistory* history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	history->instance_count++;
}

void BranchExperiment::possible_exits_helper(set<pair<int, int>>& s_possible_exits,
											 int curr_exit_depth,
											 ScopeHistory* scope_history) {
	for (int i_index = scope_history->experiment_iter_index + 1; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = scope_history->experiment_node_index + 1; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			int node_id = scope_history->node_histories[i_index][h_index]->node->id;
			s_possible_exits.insert({curr_exit_depth, node_id});
		}
	}

	if (curr_exit_depth > 0) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history
			->node_histories[scope_history->experiment_iter_index][scope_history->experiment_node_index];
		possible_exits_helper(curr_exit_depth-1,
							  scope_node_history->inner_scope_history);
	}
}

void BranchExperiment::train_existing_backprop(double target_val,
											   BranchExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		int num_instances = (int)this->i_target_val_histories.size();

		vector<map<int, vector<double>>> p_input_state_vals(this->scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, StateStatus>::iterator m_it = this->i_input_state_vals_histories[i_index][c_index].begin();
						m_it != this->i_input_state_vals_histories[i_index][c_index].end(); m_it++) {
					map<int, vector<double>>::iterator p_it = p_input_state_vals[c_index].find(m_it->first);
					if (p_it == p_input_state_vals[c_index].end()) {
						p_it = p_input_state_vals[c_index].insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[i_index] = normalized;
					} else {
						p_it->second[i_index] = m_it->second.val;
					}
				}
			}
		}
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			Scope* scope = solution->scopes[this->scope_context[c_index]];
			ScopeNode* scope_node = scope->nodes[this->node_context[c_index]];

			map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
			while (it != p_input_state_vals[c_index].end()) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_STATE
							&& scope_node->input_inner_layers[i_index] == 0
							&& !scope_node->input_outer_is_local[i_index]
							&& scope_node->input_outer_indexes[i_index] == it->first) {
						passed_down = true;
						break;
					}
				}

				if (passed_down) {
					it = p_input_state_vals[c_index].erase(it);
				} else {
					it++;
				}
			}
		}

		vector<map<int, vector<double>>> p_local_state_vals(this->scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, StateStatus>::iterator m_it = this->i_local_state_vals_histories[i_index][c_index].begin();
						m_it != this->i_local_state_vals_histories[i_index][c_index].end(); m_it++) {
					map<int, vector<double>>::iterator p_it = p_local_state_vals[c_index].find(m_it->first);
					if (p_it == p_local_state_vals[c_index].end()) {
						p_it = p_local_state_vals[c_index].insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[i_index] = normalized;
					} else {
						p_it->second[i_index] = m_it->second.val;
					}
				}
			}
		}
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			Scope* scope = solution->scopes[this->scope_context[c_index]];
			ScopeNode* scope_node = scope->nodes[this->node_context[c_index]];

			map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
			while (it != p_local_state_vals[c_index].end()) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_STATE
							&& scope_node->input_inner_layers[i_index] == 0
							&& scope_node->input_outer_is_local[i_index]
							&& scope_node->input_outer_indexes[i_index] == it->first) {
						passed_down = true;
						break;
					}
				}

				if (passed_down) {
					it = p_local_state_vals[c_index].erase(it);
				} else {
					it++;
				}
			}
		}

		map<State*, vector<double>> p_temp_state_vals;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
						m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
					map<State*, vector<double>>::iterator p_it = p_temp_state_vals.find(m_it->first);
					if (p_it == p_temp_state_vals.end()) {
						p_it = p_temp_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[i_index] = normalized;
					} else {
						p_it->second[i_index] = m_it->second.val;
					}
				}
			}
		}

		int stride_size = 0;
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			stride_size += p_input_state_vals[c_index].size();
		}
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			stride_size += p_local_state_vals[c_index].size();
		}
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			stride_size += p_temp_state_vals[c_index].size();
		}

		Eigen::MatrixXd inputs(num_instances, stride_size);
		for (int i_index = 0; i_index < num_instances; i_index++) {
			int s_index = 0;

			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
						it != p_input_state_vals[c_index].end(); it++) {
					inputs(i_index, s_index) = it->second[i_index];
					s_index++;
				}
			}

			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
						it != p_local_state_vals[c_index].end(); it++) {
					inputs(i_index, s_index) = it->second[i_index];
					s_index++;
				}
			}

			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
						it != p_temp_state_vals[c_index].end(); it++) {
					inputs(i_index, s_index) = it->second[i_index];
					s_index++;
				}
			}
		}

		Eigen::VectorXd outputs(num_instances);
		for (int i_index = 0; i_index < num_instances; i_index++) {
			outputs(i_index) = this->i_target_val_histories[i_index] - this->existing_average_score;
		}

		Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		{
			int s_index = 0;

			this->existing_input_state_weights.clear();
			this->existing_input_state_weights = vector<map<int, double>>(this->scope_context.size());
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
						it != p_input_state_vals[c_index].end(); it++) {
					this->existing_input_state_weights[c_index][it->first] = weights(s_index);
					s_index++;
				}
			}

			this->existing_local_state_weights.clear();
			this->existing_local_state_weights = vector<map<int, double>>(this->scope_context.size());
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
						it != p_local_state_vals[c_index].end(); it++) {
					this->existing_local_state_weights[c_index][it->first] = weights(s_index);
					s_index++;
				}
			}

			this->existing_temp_state_weights.clear();
			this->existing_temp_state_weights = vector<map<State*, double>>(this->scope_context.size());
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
						it != p_temp_state_vals[c_index].end(); it++) {
					this->existing_temp_state_weights[c_index][it->first] = weights(s_index);
					s_index++;
				}
			}
		}

		Eigen::VectorXd predicted_scores = inputs * weights;
		Eigen::VectorXd diffs = outputs - predicted_scores;
		vector<double> obs_experiment_target_vals(num_instances);
		for (int i_index = 0; i_index < num_instances; i_index++) {
			obs_experiment_target_vals[i_index] = diffs(i_index);
		}

		existing_obs_experiment(this,
								solution->scopes[this->scope_context[0]],
								i_scope_histories,
								obs_experiment_target_vals);

		set<pair<int, int>> s_possible_exits;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			possible_exits_helper(s_possible_exits,
								  this->scope_context.size()-1,
								  this->i_scope_histories[i_index]);
		}
		this->possible_exits.reserve(s_possible_exits.size());
		for (set<pair<int, int>>::iterator it = s_possible_exits.begin();
				it != s_possible_exits.end(); it++) {
			this->possible_exits.push_back(*it);
		}
		if (this->parent_pass_through_experiment != NULL) {
			for (int s_index = this->parent_pass_through_experiment->branch_experiment_step_index+1;
					s_index < (int)this->parent_pass_through_experiment->best_step_types.size(); s_index++) {
				if (this->parent_pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
					int node_id = this->parent_pass_through_experiment->best_actions[s_index]->id;
					s_possible_exits.insert({0, node_id});
				} else {
					int node_id = this->parent_pass_through_experiment->best_sequences[s_index]->scope_node_id;
					s_possible_exits.insert({0, node_id});
				}
			}
		}

		this->o_target_val_histories.clear();
		for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
			delete this->i_scope_histories[i_index];
		}
		this->i_scope_histories.clear();
		this->i_input_state_vals_histories.clear();
		this->i_local_state_vals_histories.clear();
		this->i_temp_state_vals_histories.clear();
		this->i_target_val_histories.clear();

		this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}