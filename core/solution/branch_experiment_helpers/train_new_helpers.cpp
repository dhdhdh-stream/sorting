#include "branch_experiment.h"

#include <iostream>

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

const int TRAIN_NEW_ITERS = 2;

const double MIN_SCORE_IMPACT = 0.05;

void BranchExperiment::train_new_target_activate(
		int& curr_node_id,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		int& exit_node_id,
		RunHelper& run_helper) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->scope_context.size()].scope_history));

	vector<map<int, StateStatus>> input_state_vals_snapshot(this->scope_context.size());
	vector<map<int, StateStatus>> local_state_vals_snapshot(this->scope_context.size());
	vecotr<map<State*, StateStatus>> temp_state_vals_snapshot(this->scope_context.size());
	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		input_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].input_state_vals;
		local_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].local_state_vals;
		temp_state_vals_snapshot[c_index] = context[context.size()-1 - c_index].temp_state_vals;
	}
	this->i_input_state_vals_histories.push_back(context.back().input_state_vals);
	this->i_local_state_vals_histories.push_back(context.back().local_state_vals);
	this->i_temp_state_vals_histories.push_back(context.back().temp_state_vals);

	PassThroughExperimentOverallHistory* history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	history->has_target = true;

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node_id
				problem,
				context,
				exit_depth,
				exit_node_id,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::train_new_non_target_activate(
		int& curr_node_id,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		int& exit_node_id,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	double original_score = this->existing_average_score;
	double branch_score = this->new_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}

			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_input_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_input_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}

			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_local_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_local_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<State*, double>::iterator original_weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}

			double branch_weight = 0.0;
			map<State*, double>::iterator branch_weight_it = this->new_temp_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_temp_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			StateNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	if (branch_predicted_score > original_predicted_score) {
		history = new BranchExperimentInstanceHistory(this);

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				history->step_indexes.push_back(s_index);
				history->step_histories.push_back(action_node_history);
				this->best_actions[s_index]->activate(
					curr_node_id
					problem,
					context,
					exit_depth,
					exit_node_id,
					run_helper,
					action_node_history);
			} else {
				SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
				history->step_indexes.push_back(s_index);
				history->step_histories.push_back(sequence_history);
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
}

void BranchExperiment::train_backprop(double target_val,
									  BranchExperimentOverallHistory* history) {
	if (history->has_target) {
		this->i_target_val_histories.push_back(target_val);

		if (this->i_target_val_histories.size() >= solution->curr_num_datapoints) {
			double sum_scores = 0.0;
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				sum_scores += this->i_target_val_histories[i_index];
			}
			this->new_average_score = sum_scores / solution->curr_num_datapoints;

			vector<map<int, vector<double>>> p_input_state_vals(this->scope_context.size());
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_input_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_input_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_input_state_vals[c_index].find(m_it->first);
						if (p_it == p_input_state_vals[c_index].end()) {
							p_it = p_input_state_vals[c_index].insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_local_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_local_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_local_state_vals[c_index].find(m_it->first);
						if (p_it == p_local_state_vals[c_index].end()) {
							p_it = p_local_state_vals[c_index].insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<State*, vector<double>>::iterator p_it = p_temp_state_vals.find(m_it->first);
						if (p_it == p_temp_state_vals.end()) {
							p_it = p_temp_state_vals.insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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

			Eigen::MatrixXd inputs(solution->curr_num_datapoints, stride_size);
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
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

			Eigen::VectorXd outputs(solution->curr_num_datapoints);
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index] - this->new_average_score;
			}

			Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			{
				int s_index = 0;

				this->new_input_state_weights.clear();
				this->new_input_state_weights = vector<map<int, double>>(this->scope_context.size());
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
							it != p_input_state_vals[c_index].end(); it++) {
						this->new_input_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				this->new_local_state_weights.clear();
				this->new_local_state_weights = vector<map<int, double>>(this->scope_context.size());
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
							it != p_local_state_vals[c_index].end(); it++) {
						this->new_local_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				this->new_temp_state_weights.clear();
				this->new_temp_state_weights = vector<map<State*, double>>(this->scope_context.size());
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
			vector<double> obs_experiment_target_vals(solution->curr_num_datapoints);
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				obs_experiment_target_vals[i_index] = diffs(i_index);
			}

			new_obs_experiment(this,
							   i_scope_histories,
							   obs_experiment_target_vals);

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_input_state_vals_histories.clear();
			this->i_local_state_vals_histories.clear();
			this->i_temp_state_vals_histories.clear();
			this->i_target_val_histories.clear();

			if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN_PRE) {
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = BRANCH_EXPERIMENT_STATE_TRAIN;
				this->state_iter = 0;
			} else {
				// this->state == BRANCH_EXPERIMENT_STATE_TRAIN
				this->state_iter++;
				if (this->state_iter >= TRAIN_NEW_ITERS) {
					double score_standard_deviation = sqrt(this->existing_score_variance);

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<int, double>::iterator existing_it = this->existing_input_state_weights[c_index].begin();
						while (existing_it != this->existing_input_state_weights[c_index].end()) {
							double existing_weight = existing_it->second;

							double new_weight = 0.0;
							map<int, double>::iterator new_it = this->new_input_state_weights[c_index].find(existing_it->first);
							if (new_it != this->new_input_state_weights[c_index].end()) {
								new_weight = new_it->second;
							}

							if (abs(existing_weight - new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
								if (new_it != this->new_input_state_weights[c_index].end()) {
									this->new_input_state_weights[c_index].erase(new_it);
								}

								existing_it = this->existing_input_state_weights[c_index].erase(existing_it);
							} else {
								if (new_it == this->new_input_state_weights[c_index].end()) {
									this->new_input_state_weights[c_index][existing_it->first] = 0.0;
								}

								existing_it++;
							}
						}

						map<int, double>::iterator new_it = this->new_input_state_weights[c_index].begin();
						while (new_it != this->new_input_state_weights[c_index].end()) {
							map<int, double>::iterator existing_it = this->existing_input_state_weights[c_index].find(new_it->first);
							if (existing_it == this->existing_input_state_weights[c_index].end()) {
								double new_weight = new_it->second;
								if (abs(new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
									new_it = this->new_input_state_weights[c_index].erase(new_it);
								} else {
									this->existing_input_state_weights[c_index][new_it->first] = 0.0;

									new_it++;
								}
							}
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<int, double>::iterator existing_it = this->existing_local_state_weights[c_index].begin();
						while (existing_it != this->existing_local_state_weights[c_index].end()) {
							double existing_weight = existing_it->second;

							double new_weight = 0.0;
							map<int, double>::iterator new_it = this->new_local_state_weights[c_index].find(existing_it->first);
							if (new_it != this->new_local_state_weights[c_index].end()) {
								new_weight = new_it->second;
							}

							if (abs(existing_weight - new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
								if (new_it != this->new_local_state_weights[c_index].end()) {
									this->new_local_state_weights[c_index].erase(new_it);
								}

								existing_it = this->existing_local_state_weights[c_index].erase(existing_it);
							} else {
								if (new_it == this->new_local_state_weights[c_index].end()) {
									this->new_local_state_weights[c_index][existing_it->first] = 0.0;
								}

								existing_it++;
							}
						}

						map<int, double>::iterator new_it = this->new_local_state_weights[c_index].begin();
						while (new_it != this->new_local_state_weights[c_index].end()) {
							map<int, double>::iterator existing_it = this->existing_local_state_weights[c_index].find(new_it->first);
							if (existing_it == this->existing_local_state_weights[c_index].end()) {
								double new_weight = new_it->second;
								if (abs(new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
									new_it = this->new_local_state_weights[c_index].erase(new_it);
								} else {
									this->existing_local_state_weights[c_index][new_it->first] = 0.0;

									new_it++;
								}
							}
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<State*, double>::iterator existing_it = this->existing_temp_state_weights[c_index].begin();
						while (existing_it != this->existing_temp_state_weights[c_index].end()) {
							double existing_weight = existing_it->second;

							double new_weight = 0.0;
							map<State*, double>::iterator new_it = this->new_temp_state_weights[c_index].find(existing_it->first);
							if (new_it != this->new_temp_state_weights[c_index].end()) {
								new_weight = new_it->second;
							}

							if (abs(existing_weight - new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
								if (new_it != this->new_temp_state_weights[c_index].end()) {
									this->new_temp_state_weights[c_index].erase(new_it);
								}

								existing_it = this->existing_temp_state_weights[c_index].erase(existing_it);
							} else {
								if (new_it == this->new_temp_state_weights[c_index].end()) {
									this->new_temp_state_weights[c_index][existing_it->first] = 0.0;
								}

								existing_it++;
							}
						}

						map<State*, double>::iterator new_it = this->new_temp_state_weights[c_index].begin();
						while (new_it != this->new_temp_state_weights[c_index].end()) {
							map<State*, double>::iterator existing_it = this->existing_temp_state_weights[c_index].find(new_it->first);
							if (existing_it == this->existing_temp_state_weights[c_index].end()) {
								double new_weight = new_it->second;
								if (abs(new_weight) < MIN_SCORE_IMPACT * score_standard_deviation) {
									new_it = this->new_temp_state_weights[c_index].erase(new_it);
								} else {
									this->existing_temp_state_weights[c_index][new_it->first] = 0.0;

									new_it++;
								}
							}
						}
					}

					this->state = BRANCH_EXPERIMENT_STATE_MEASURE;
					this->state_iter = 0;
				} else {
					this->i_scope_histories.reserve(solution->curr_num_datapoints);
					this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
					this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
					this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
					this->i_target_val_histories.reserve(solution->curr_num_datapoints);
				}
			}
		}
	}
}
