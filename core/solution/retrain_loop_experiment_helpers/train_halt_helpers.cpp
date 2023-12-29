#include "retrain_loop_experiment.h"

#include <Eigen/Dense>

#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void RetrainLoopExperiment::train_halt_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	RetrainLoopExperimentOverallHistory* overall_history = (RetrainLoopExperimentOverallHistory*)run_helper.experiment_history;

	int iter_index = 0;
	while (true) {
		this->i_scope_histories.push_back(new ScopeHistory(context[context.size() - 1 - this->scope_node->loop_scope_context.size()].scope_history));

		vector<map<int, StateStatus>> input_state_vals_snapshot(this->scope_node->loop_scope_context.size());
		vector<map<int, StateStatus>> local_state_vals_snapshot(this->scope_node->loop_scope_context.size());
		vector<map<State*, StateStatus>> temp_state_vals_snapshot(this->scope_node->loop_scope_context.size());
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			input_state_vals_snapshot[c_index] = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals;
			local_state_vals_snapshot[c_index] = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals;
			temp_state_vals_snapshot[c_index] = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals;
		}
		this->i_input_state_vals_histories.push_back(input_state_vals_snapshot);
		this->i_local_state_vals_histories.push_back(local_state_vals_snapshot);
		this->i_temp_state_vals_histories.push_back(temp_state_vals_snapshot);

		overall_history->num_iters++;

		if (iter_index >= this->scope_node->max_iters) {
			break;
		}

		/**
		 * - capture halt_score without this->scope_node->halt_score_mod
		 */
		double halt_score = 0.0;

		for (int s_index = 0; s_index < (int)this->scope_node->loop_state_is_local.size(); s_index++) {
			if (this->scope_node->loop_state_is_local[s_index]) {
				map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
				if (it != context[context.size()-2].local_state_vals.end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += this->scope_node->loop_halt_weights[s_index] * normalized;
					} else {
						halt_score += this->scope_node->loop_halt_weights[s_index] * it->second.val;
					}
				}
			} else {
				map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
				if (it != context[context.size()-2].input_state_vals.end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += this->scope_node->loop_halt_weights[s_index] * normalized;
					} else {
						halt_score += this->scope_node->loop_halt_weights[s_index] * it->second.val;
					}
				}
			}
		}

		this->i_target_val_histories.push_back(halt_score);

		this->scope_node->inner_scope->activate(problem,
												context,
												inner_exit_depth,
												inner_exit_node,
												run_helper,
												iter_index,
												parent_scope_node_history->inner_scope_history);

		for (int o_index = 0; o_index < (int)this->scope_node->output_inner_indexes.size(); o_index++) {
			if (this->scope_node->output_inner_is_local[o_index]) {
				map<int, StateStatus>::iterator inner_it = context.back().local_state_vals.find(this->scope_node->output_inner_indexes[o_index]);
				if (inner_it != context.back().local_state_vals.end()) {
					if (this->scope_node->output_outer_is_local[o_index]) {
						context[context.size()-2].local_state_vals[this->scope_node->output_outer_indexes[o_index]] = inner_it->second;
					} else {
						map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->scope_node->output_outer_indexes[o_index]);
						if (outer_it != context[context.size()-2].input_state_vals.end()) {
							outer_it->second = inner_it->second;
						}
					}
				}
			} else {
				map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->scope_node->output_inner_indexes[o_index]);
				if (inner_it != context.back().input_state_vals.end()) {
					if (this->scope_node->output_outer_is_local[o_index]) {
						context[context.size()-2].local_state_vals[this->scope_node->output_outer_indexes[o_index]] = inner_it->second;
					} else {
						map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->scope_node->output_outer_indexes[o_index]);
						if (outer_it != context[context.size()-2].input_state_vals.end()) {
							outer_it->second = inner_it->second;
						}
					}
				}
			}
		}

		if (inner_exit_depth != -1
				|| run_helper.exceeded_limit) {
			break;
		} else {
			iter_index++;
			// continue
		}
	}
}

void RetrainLoopExperiment::train_halt_backprop(
		double target_val,
		RetrainLoopExperimentOverallHistory* history) {
	if (history->num_iters == this->scope_node->max_iters) {
		this->i_target_val_histories.push_back(target_val - this->scope_node->halt_score_mod);
	}

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		int num_instances = (int)this->i_target_val_histories.size();

		vector<map<int, vector<double>>> p_input_state_vals(this->scope_node->loop_scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
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
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size()-1; c_index++) {
			Scope* scope = solution->scopes[this->scope_node->loop_scope_context[c_index]];
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->scope_node->loop_node_context[c_index]];

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

		vector<map<int, vector<double>>> p_local_state_vals(this->scope_node->loop_scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
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
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size()-1; c_index++) {
			Scope* scope = solution->scopes[this->scope_node->loop_scope_context[c_index]];
			ScopeNode* scope_node = (ScopeNode*)scope->nodes[this->scope_node->loop_node_context[c_index]];

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

		vector<map<State*, vector<double>>> p_temp_state_vals(this->scope_node->loop_scope_context.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
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
		for (int c_index = 1; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
			while (it != p_temp_state_vals[c_index].end()) {
				bool is_new_state = false;
				for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
					if (it->first == this->new_states[s_index]) {
						is_new_state = true;
						break;
					}
				}

				if (is_new_state) {
					it = p_temp_state_vals[c_index].erase(it);
				} else {
					it++;
				}
			}
		}

		int stride_size = 0;
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			stride_size += p_input_state_vals[c_index].size();
		}
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			stride_size += p_local_state_vals[c_index].size();
		}
		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			stride_size += p_temp_state_vals[c_index].size();
		}

		this->halt_input_state_weights = vector<map<int, double>>(this->scope_node->loop_scope_context.size());
		this->halt_local_state_weights = vector<map<int, double>>(this->scope_node->loop_scope_context.size());
		this->halt_temp_state_weights = vector<map<State*, double>>(this->scope_node->loop_scope_context.size());
		vector<double> obs_experiment_target_vals(num_instances);
		if (stride_size > 0) {
			Eigen::MatrixXd inputs(num_instances, stride_size);
			for (int i_index = 0; i_index < num_instances; i_index++) {
				int s_index = 0;

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
							it != p_input_state_vals[c_index].end(); it++) {
						inputs(i_index, s_index) = it->second[i_index];
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
							it != p_local_state_vals[c_index].end(); it++) {
						inputs(i_index, s_index) = it->second[i_index];
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
							it != p_temp_state_vals[c_index].end(); it++) {
						inputs(i_index, s_index) = it->second[i_index];
						s_index++;
					}
				}
			}

			Eigen::VectorXd outputs(num_instances);
			for (int i_index = 0; i_index < num_instances; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index];
			}

			Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			{
				int s_index = 0;

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
							it != p_input_state_vals[c_index].end(); it++) {
						this->halt_input_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
							it != p_local_state_vals[c_index].end(); it++) {
						this->halt_local_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
					for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
							it != p_temp_state_vals[c_index].end(); it++) {
						this->halt_temp_state_weights[c_index][it->first] = weights(s_index);
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
				obs_experiment_target_vals[i_index] = this->i_target_val_histories[i_index];
			}
		}

		existing_obs_experiment(this,
								solution->scopes[this->scope_node->loop_scope_context[0]],
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

		this->state = RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE;
		this->state_iter = 0;
		this->sub_state_iter = 0;
	}
}
