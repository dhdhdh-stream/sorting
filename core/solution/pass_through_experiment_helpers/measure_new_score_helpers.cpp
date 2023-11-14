#include "pass_through_experiment.h"

#include <iostream>
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

void PassThroughExperiment::measure_new_score_activate(
		AbstractNode*& curr_node,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	PassThroughExperimentInstanceHistory* instance_history = new PassThroughExperimentInstanceHistory(this);
	history = instance_history;

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			instance_history->pre_step_histories.push_back(action_node_history);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			instance_history->pre_step_histories.push_back(sequence_history);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node = this->best_exit_node;
	} else {
		curr_node = NULL;

		exit_depth = this->best_exit_depth-1;
		exit_node = this->best_exit_node;
	}

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

	PassThroughExperimentOverallHistory* overall_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
}

void PassThroughExperiment::measure_new_score_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / solution->curr_num_datapoints;

		double score_improvement = this->new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		if (score_improvement_t_score > 2.326) {	// >99%
			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_EXISTING_SCORE;
			this->state_iter = 0;
		} else if (this->best_step_types.size() > 0 && score_improvement_t_score > -0.674) {	// <75%
			/**
			 * - TODO: consider other conditions
			 *   - e.g., small number of particularly high scoring instances
			 */

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
				ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->node_context[c_index]];

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
				ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->node_context[c_index]];

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

			vector<map<State*, vector<double>>> p_temp_state_vals(this->scope_context.size());
			for (int i_index = 0; i_index < num_instances; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<State*, vector<double>>::iterator p_it = p_temp_state_vals[c_index].find(m_it->first);
						if (p_it == p_temp_state_vals[c_index].end()) {
							p_it = p_temp_state_vals[c_index].insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
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

			this->new_input_state_weights = vector<map<int, double>>(this->scope_context.size());
			this->new_local_state_weights = vector<map<int, double>>(this->scope_context.size());
			this->new_temp_state_weights = vector<map<State*, double>>(this->scope_context.size());
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
					outputs(i_index) = this->i_target_val_histories[i_index] - this->new_average_score;
				}

				Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				{
					int s_index = 0;

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
								it != p_input_state_vals[c_index].end(); it++) {
							this->new_input_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
								it != p_local_state_vals[c_index].end(); it++) {
							this->new_local_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
								it != p_temp_state_vals[c_index].end(); it++) {
							this->new_temp_state_weights[c_index][it->first] = weights(s_index);
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
					obs_experiment_target_vals[i_index] = this->i_target_val_histories[i_index] - this->new_average_score;
				}
			}

			new_obs_experiment(this,
							   i_scope_histories,
							   obs_experiment_target_vals);

			// reserve at least solution->curr_num_datapoints
			this->i_misguess_histories.reserve(solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS;
			this->state_iter = 0;
		} else {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_sequences[s_index];
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();

			for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
				delete this->new_states[s_index];
			}
			this->new_states.clear();

			this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
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

		cout << endl;
	}
}
