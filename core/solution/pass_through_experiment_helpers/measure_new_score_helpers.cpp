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

		cout << "PassThrough" << endl;
		cout << "this->scope_context:" << endl;
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			cout << c_index << ": " << this->scope_context[c_index] << endl;
		}
		cout << "this->node_context:" << endl;
		for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
			cout << c_index << ": " << this->node_context[c_index] << endl;
		}
		cout << "new explore path:";
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				cout << " " << this->best_actions[s_index]->action.to_string();
			} else {
				cout << " S";
			}
		}
		cout << endl;

		cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
		if (this->best_exit_node == NULL) {
			cout << "this->best_exit_node_id: " << -1 << endl;
		} else {
			cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
		}

		double score_improvement = this->new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << "this->new_average_score: " << this->new_average_score << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		if (score_improvement_t_score > 2.326) {	// >99%
			cout << "score success" << endl;

			Scope* containing_scope = solution->scopes[this->scope_context.back()];

			BranchNode* new_branch_node = new BranchNode();
			new_branch_node->parent = containing_scope;
			new_branch_node->id = containing_scope->node_counter;
			containing_scope->node_counter++;
			containing_scope->nodes[new_branch_node->id] = new_branch_node;

			ExitNode* new_exit_node = new ExitNode();
			new_exit_node->parent = containing_scope;
			new_exit_node->id = containing_scope->node_counter;
			containing_scope->node_counter++;
			containing_scope->nodes[new_exit_node->id] = new_exit_node;

			new_branch_node->branch_scope_context = this->scope_context;
			new_branch_node->branch_node_context = this->node_context;
			new_branch_node->branch_node_context.back() = new_branch_node->id;

			new_branch_node->branch_is_pass_through = true;

			new_branch_node->original_score_mod = 0.0;
			new_branch_node->branch_score_mod = 0.0;

			if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

				new_branch_node->original_next_node_id = action_node->next_node_id;
				new_branch_node->original_next_node = action_node->next_node;

				action_node->next_node_id = new_branch_node->id;
				action_node->next_node = new_branch_node;
			} else {
				ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

				new_branch_node->original_next_node_id = scope_node->next_node_id;
				new_branch_node->original_next_node = scope_node->next_node;

				scope_node->next_node_id = new_branch_node->id;
				scope_node->next_node = new_branch_node;
			}

			if (this->best_step_types.size() == 0) {
				new_branch_node->branch_next_node_id = new_exit_node->id;
				new_branch_node->branch_next_node = new_exit_node;
			} else if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				new_branch_node->branch_next_node_id = this->best_actions[0]->id;
				new_branch_node->branch_next_node = this->best_actions[0];
			} else {
				new_branch_node->branch_next_node_id = this->best_sequences[0]->scope_node_placeholder->id;
				new_branch_node->branch_next_node = this->best_sequences[0]->scope_node_placeholder;
			}

			map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
			map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				AbstractNode* next_node;
				if (s_index == (int)this->best_step_types.size()-1) {
					next_node = new_exit_node;
				} else {
					if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
						next_node = this->best_actions[s_index+1];
					} else {
						next_node = this->best_sequences[s_index+1]->scope_node_placeholder;
					}
				}

				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					containing_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

					this->best_actions[s_index]->next_node_id = next_node->id;
					this->best_actions[s_index]->next_node = next_node;
				} else {
					finalize_sequence(this->scope_context,
									  this->node_context,
									  this->best_sequences[s_index],
									  input_scope_depths_mappings,
									  output_scope_depths_mappings);
					ScopeNode* new_sequence_scope_node = this->best_sequences[s_index]->scope_node_placeholder;
					this->best_sequences[s_index]->scope_node_placeholder = NULL;
					containing_scope->nodes[new_sequence_scope_node->id] = new_sequence_scope_node;

					new_sequence_scope_node->next_node_id = next_node->id;
					new_sequence_scope_node->next_node = next_node;

					delete this->best_sequences[s_index];

					containing_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();

			new_exit_node->exit_depth = this->best_exit_depth;
			if (this->best_exit_node == NULL) {
				new_exit_node->exit_node_parent_id = -1;
				new_exit_node->exit_node_id = -1;
			} else {
				new_exit_node->exit_node_parent_id = this->best_exit_node->parent->id;
				new_exit_node->exit_node_id = this->best_exit_node->id;
			}
			new_exit_node->exit_node = this->best_exit_node;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_SUCCESS;
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
