#include "retrain_branch_experiment.h"

#include <Eigen/Dense>

#include "branch_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "solution_helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void RetrainBranchExperiment::train_branch_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	bool is_target = false;
	RetrainBranchExperimentOverallHistory* overall_history = (RetrainBranchExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->instance_count++;
	if (!overall_history->has_target) {
		double target_probability;
		if (overall_history->instance_count > this->average_instances_per_run) {
			target_probability = 0.5;
		} else {
			target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - overall_history->instance_count));
		}
		uniform_real_distribution<double> distribution(0.0, 1.0);
		if (distribution(generator) < target_probability) {
			is_target = true;
		}
	}

	if (is_target) {
		overall_history->has_target = true;

		train_branch_target_activate(is_branch,
									 context);
	} else {
		train_branch_non_target_activate(is_branch,
										 context,
										 run_helper);
	}
}

void RetrainBranchExperiment::train_branch_target_activate(
		bool& is_branch,
		vector<ContextLayer>& context) {
	this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - this->branch_node->branch_scope_context.size()].scope_history));

	vector<map<int, StateStatus>> input_state_vals_snapshot(this->branch_node->branch_scope_context.size());
	vector<map<int, StateStatus>> local_state_vals_snapshot(this->branch_node->branch_scope_context.size());
	vector<map<State*, StateStatus>> temp_state_vals_snapshot(this->branch_node->branch_scope_context.size());
	for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
		input_state_vals_snapshot[c_index] = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].input_state_vals;
		local_state_vals_snapshot[c_index] = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].local_state_vals;
		temp_state_vals_snapshot[c_index] = context[context.size() - this->branch_node->branch_scope_context.size() + c_index].temp_state_vals;
	}
	this->i_input_state_vals_histories.push_back(input_state_vals_snapshot);
	this->i_local_state_vals_histories.push_back(local_state_vals_snapshot);
	this->i_temp_state_vals_histories.push_back(temp_state_vals_snapshot);

	is_branch = true;
}

void RetrainBranchExperiment::train_branch_non_target_activate(
		bool& is_branch,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	double original_score = this->branch_node->original_score_mod;
	double branch_score = this->branch_node->branch_score_mod;

	for (int s_index = 0; s_index < (int)this->branch_node->decision_state_is_local.size(); s_index++) {
		if (this->branch_node->decision_state_is_local[s_index]) {
			map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->branch_node->decision_state_indexes[s_index]);
			if (it != context.back().local_state_vals.end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					original_score += this->branch_node->decision_original_weights[s_index] * normalized;
					branch_score += this->branch_node->decision_branch_weights[s_index] * normalized;
				} else {
					original_score += this->branch_node->decision_original_weights[s_index] * it->second.val;
					branch_score += this->branch_node->decision_branch_weights[s_index] * it->second.val;
				}
			}
		} else {
			map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->branch_node->decision_state_indexes[s_index]);
			if (it != context.back().input_state_vals.end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					original_score += this->branch_node->decision_original_weights[s_index] * normalized;
					branch_score += this->branch_node->decision_branch_weights[s_index] * normalized;
				} else {
					original_score += this->branch_node->decision_original_weights[s_index] * it->second.val;
					branch_score += this->branch_node->decision_branch_weights[s_index] * it->second.val;
				}
			}
		}
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch;
	if (abs(branch_score - original_score) > DECISION_MIN_SCORE_IMPACT * this->branch_node->decision_standard_deviation) {
		decision_is_branch = branch_score > original_score;
	} else {
		uniform_int_distribution<int> distribution(0, 1);
		decision_is_branch = distribution(generator);
	}
	#endif /* MDEBUG */

	if (decision_is_branch) {
		is_branch = true;
	} else {
		is_branch = false;
	}
}

void RetrainBranchExperiment::train_branch_backprop(
		double target_val,
		RetrainBranchExperimentOverallHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		this->i_target_val_histories.push_back(target_val);

		if ((int)this->i_target_val_histories.size() >= solution->curr_num_datapoints) {
			double sum_scores = 0.0;
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				sum_scores += this->i_target_val_histories[i_index];
			}
			this->branch_average_score = sum_scores / solution->curr_num_datapoints;

			vector<map<int, vector<double>>> p_input_state_vals(this->branch_node->branch_scope_context.size());
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_input_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_input_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_input_state_vals[c_index].find(m_it->first);
						if (p_it == p_input_state_vals[c_index].end()) {
							p_it = p_input_state_vals[c_index].insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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
			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size()-1; c_index++) {
				Scope* scope = solution->scopes[this->branch_node->branch_scope_context[c_index]];
				ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->branch_node->branch_node_context[c_index]];

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

			vector<map<int, vector<double>>> p_local_state_vals(this->branch_node->branch_scope_context.size());
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_local_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_local_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_local_state_vals[c_index].find(m_it->first);
						if (p_it == p_local_state_vals[c_index].end()) {
							p_it = p_local_state_vals[c_index].insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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
			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size()-1; c_index++) {
				Scope* scope = solution->scopes[this->branch_node->branch_scope_context[c_index]];
				ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->branch_node->branch_node_context[c_index]];

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

			vector<map<State*, vector<double>>> p_temp_state_vals(this->branch_node->branch_scope_context.size());
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
					for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<State*, vector<double>>::iterator p_it = p_temp_state_vals[c_index].find(m_it->first);
						if (p_it == p_temp_state_vals[c_index].end()) {
							p_it = p_temp_state_vals[c_index].insert({m_it->first, vector<double>(solution->curr_num_datapoints, 0.0)}).first;
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
			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				stride_size += p_input_state_vals[c_index].size();
			}
			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				stride_size += p_local_state_vals[c_index].size();
			}
			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				stride_size += p_temp_state_vals[c_index].size();
			}

			this->branch_input_state_weights = vector<map<int, double>>(this->branch_node->branch_scope_context.size());
			this->branch_local_state_weights = vector<map<int, double>>(this->branch_node->branch_scope_context.size());
			this->branch_temp_state_weights = vector<map<State*, double>>(this->branch_node->branch_scope_context.size());
			vector<double> obs_experiment_target_vals(solution->curr_num_datapoints);
			if (stride_size > 0) {
				Eigen::MatrixXd inputs(solution->curr_num_datapoints, stride_size);
				for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
					int s_index = 0;

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
								it != p_input_state_vals[c_index].end(); it++) {
							inputs(i_index, s_index) = it->second[i_index];
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
								it != p_local_state_vals[c_index].end(); it++) {
							inputs(i_index, s_index) = it->second[i_index];
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
								it != p_temp_state_vals[c_index].end(); it++) {
							inputs(i_index, s_index) = it->second[i_index];
							s_index++;
						}
					}
				}

				Eigen::VectorXd outputs(solution->curr_num_datapoints);
				for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
					outputs(i_index) = this->i_target_val_histories[i_index] - this->branch_average_score;
				}

				Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				{
					int s_index = 0;

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
								it != p_input_state_vals[c_index].end(); it++) {
							this->branch_input_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
								it != p_local_state_vals[c_index].end(); it++) {
							this->branch_local_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
						for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
								it != p_temp_state_vals[c_index].end(); it++) {
							this->branch_temp_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}
				}

				Eigen::VectorXd predicted_scores = inputs * weights;
				Eigen::VectorXd diffs = outputs - predicted_scores;
				for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
					obs_experiment_target_vals[i_index] = diffs(i_index);
				}
			} else {
				for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
					obs_experiment_target_vals[i_index] = this->i_target_val_histories[i_index] - this->branch_average_score;
				}
			}

			existing_obs_experiment(this,
									solution->scopes[this->branch_node->branch_scope_context[0]],
									this->i_scope_histories,
									obs_experiment_target_vals);

			for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
				delete this->i_scope_histories[i_index];
			}
			this->i_scope_histories.clear();
			this->i_input_state_vals_histories.clear();
			this->i_local_state_vals_histories.clear();
			this->i_temp_state_vals_histories.clear();
			this->i_target_val_histories.clear();

			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				map<int, double>::iterator original_it = this->original_input_state_weights[c_index].begin();
				while (original_it != this->original_input_state_weights[c_index].end()) {
					double original_weight = original_it->second;

					double branch_weight = 0.0;
					map<int, double>::iterator branch_it = this->branch_input_state_weights[c_index].find(original_it->first);
					if (branch_it != this->branch_input_state_weights[c_index].end()) {
						branch_weight = branch_it->second;
					}

					if (abs(original_weight - branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
						if (branch_it != this->branch_input_state_weights[c_index].end()) {
							this->branch_input_state_weights[c_index].erase(branch_it);
						}

						original_it = this->original_input_state_weights[c_index].erase(original_it);
					} else {
						if (branch_it == this->branch_input_state_weights[c_index].end()) {
							this->branch_input_state_weights[c_index][original_it->first] = 0.0;
						}

						original_it++;
					}
				}

				map<int, double>::iterator branch_it = this->branch_input_state_weights[c_index].begin();
				while (branch_it != this->branch_input_state_weights[c_index].end()) {
					map<int, double>::iterator original_it = this->original_input_state_weights[c_index].find(branch_it->first);
					if (original_it == this->original_input_state_weights[c_index].end()) {
						double branch_weight = branch_it->second;
						if (abs(branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
							branch_it = this->branch_input_state_weights[c_index].erase(branch_it);
						} else {
							this->original_input_state_weights[c_index][branch_it->first] = 0.0;

							branch_it++;
						}
					} else {
						branch_it++;
					}
				}
			}

			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				map<int, double>::iterator original_it = this->original_local_state_weights[c_index].begin();
				while (original_it != this->original_local_state_weights[c_index].end()) {
					double original_weight = original_it->second;

					double branch_weight = 0.0;
					map<int, double>::iterator branch_it = this->branch_local_state_weights[c_index].find(original_it->first);
					if (branch_it != this->branch_local_state_weights[c_index].end()) {
						branch_weight = branch_it->second;
					}

					if (abs(original_weight - branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
						if (branch_it != this->branch_local_state_weights[c_index].end()) {
							this->branch_local_state_weights[c_index].erase(branch_it);
						}

						original_it = this->original_local_state_weights[c_index].erase(original_it);
					} else {
						if (branch_it == this->branch_local_state_weights[c_index].end()) {
							this->branch_local_state_weights[c_index][original_it->first] = 0.0;
						}

						original_it++;
					}
				}

				map<int, double>::iterator branch_it = this->branch_local_state_weights[c_index].begin();
				while (branch_it != this->branch_local_state_weights[c_index].end()) {
					map<int, double>::iterator original_it = this->original_local_state_weights[c_index].find(branch_it->first);
					if (original_it == this->original_local_state_weights[c_index].end()) {
						double branch_weight = branch_it->second;
						if (abs(branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
							branch_it = this->branch_local_state_weights[c_index].erase(branch_it);
						} else {
							this->original_local_state_weights[c_index][branch_it->first] = 0.0;

							branch_it++;
						}
					} else {
						branch_it++;
					}
				}
			}

			for (int c_index = 0; c_index < (int)this->branch_node->branch_scope_context.size(); c_index++) {
				map<State*, double>::iterator original_it = this->original_temp_state_weights[c_index].begin();
				while (original_it != this->original_temp_state_weights[c_index].end()) {
					double original_weight = original_it->second;

					double branch_weight = 0.0;
					map<State*, double>::iterator branch_it = this->branch_temp_state_weights[c_index].find(original_it->first);
					if (branch_it != this->branch_temp_state_weights[c_index].end()) {
						branch_weight = branch_it->second;
					}

					if (abs(original_weight - branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
						if (branch_it != this->branch_temp_state_weights[c_index].end()) {
							this->branch_temp_state_weights[c_index].erase(branch_it);
						}

						original_it = this->original_temp_state_weights[c_index].erase(original_it);
					} else {
						if (branch_it == this->branch_temp_state_weights[c_index].end()) {
							this->branch_temp_state_weights[c_index][original_it->first] = 0.0;
						}

						original_it++;
					}
				}

				map<State*, double>::iterator branch_it = this->branch_temp_state_weights[c_index].begin();
				while (branch_it != this->branch_temp_state_weights[c_index].end()) {
					map<State*, double>::iterator original_it = this->original_temp_state_weights[c_index].find(branch_it->first);
					if (original_it == this->original_temp_state_weights[c_index].end()) {
						double branch_weight = branch_it->second;
						if (abs(branch_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
							branch_it = this->branch_temp_state_weights[c_index].erase(branch_it);
						} else {
							this->original_temp_state_weights[c_index][branch_it->first] = 0.0;

							branch_it++;
						}
					} else {
						branch_it++;
					}
				}
			}

			this->state = RETRAIN_BRANCH_EXPERIMENT_STATE_MEASURE;
			this->state_iter = 0;
		}
	}
}
