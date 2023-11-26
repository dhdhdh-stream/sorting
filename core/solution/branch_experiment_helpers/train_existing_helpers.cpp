#include "branch_experiment.h"

#include <iostream>
#include <stdexcept>
#include <Eigen/Dense>

#include "action_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "pass_through_experiment.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void BranchExperiment::train_existing_activate(vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	vector<map<int, StateStatus>> input_state_vals_snapshot(this->scope_context.size());
	vector<map<int, StateStatus>> local_state_vals_snapshot(this->scope_context.size());
	vector<map<State*, StateStatus>> temp_state_vals_snapshot(this->scope_context.size());
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		input_state_vals_snapshot[c_index] = context[context.size() - this->scope_context.size() + c_index].input_state_vals;
		local_state_vals_snapshot[c_index] = context[context.size() - this->scope_context.size() + c_index].local_state_vals;
		temp_state_vals_snapshot[c_index] = context[context.size() - this->scope_context.size() + c_index].temp_state_vals;
	}
	this->i_input_state_vals_histories.push_back(input_state_vals_snapshot);
	this->i_local_state_vals_histories.push_back(local_state_vals_snapshot);
	this->i_temp_state_vals_histories.push_back(temp_state_vals_snapshot);

	BranchExperimentOverallHistory* overall_history;
	if (this->parent_pass_through_experiment != NULL) {
		PassThroughExperimentOverallHistory* parent_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
		overall_history = parent_history->branch_experiment_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)run_helper.experiment_history;
	}
	overall_history->instance_count++;
}

void BranchExperiment::train_existing_backprop(double target_val,
											   RunHelper& run_helper,
											   BranchExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (this->parent_pass_through_experiment == NULL) {
		if (!run_helper.exceeded_depth) {
			if (run_helper.max_depth > solution->max_depth) {
				solution->max_depth = run_helper.max_depth;

				if (solution->max_depth < 50) {
					solution->depth_limit = solution->max_depth + 10;
				} else {
					solution->depth_limit = (int)(1.2*(double)solution->max_depth);
				}
			}
		}
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		cout << "Branch" << endl;
		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << endl;

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

					FullNetwork* last_network = m_it->second.last_network;
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
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->node_context[c_index]];

			map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
			while (it != p_input_state_vals[c_index].end()) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_STATE
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

					FullNetwork* last_network = m_it->second.last_network;
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
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->node_context[c_index]];

			map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
			while (it != p_local_state_vals[c_index].end()) {
				bool passed_down = false;
				for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
					if (scope_node->input_types[i_index] == INPUT_TYPE_STATE
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

		vector<map<State*, vector<double>>> p_temp_state_vals(this->scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
						m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
					map<State*, vector<double>>::iterator p_it = p_temp_state_vals[c_index].find(m_it->first);
					if (p_it == p_temp_state_vals[c_index].end()) {
						p_it = p_temp_state_vals[c_index].insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					FullNetwork* last_network = m_it->second.last_network;
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

		this->existing_input_state_weights = vector<map<int, double>>(this->scope_context.size());
		this->existing_local_state_weights = vector<map<int, double>>(this->scope_context.size());
		this->existing_temp_state_weights = vector<map<State*, double>>(this->scope_context.size());
		vector<double> obs_experiment_target_vals(num_instances);
		if (stride_size > 0) {
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

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
							it != p_input_state_vals[c_index].end(); it++) {
						this->existing_input_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
							it != p_local_state_vals[c_index].end(); it++) {
						this->existing_local_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

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
			for (int i_index = 0; i_index < num_instances; i_index++) {
				obs_experiment_target_vals[i_index] = diffs(i_index);
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				obs_experiment_target_vals[i_index] = this->i_target_val_histories[i_index] - this->existing_average_score;
			}
		}

		if (this->parent_pass_through_experiment == NULL) {
			existing_obs_experiment(this,
									solution->scopes[this->scope_context[0]],
									i_scope_histories,
									obs_experiment_target_vals);
		} else {
			existing_pass_through_branch_obs_experiment(
				this,
				i_scope_histories,
				obs_experiment_target_vals);
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
