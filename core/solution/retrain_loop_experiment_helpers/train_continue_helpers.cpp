#include "retrain_loop_experiment.h"

#include <Eigen/Dense>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int TRAIN_CONTINUE_ITERS = 2;

void RetrainLoopExperiment::train_continue_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	bool is_target = false;
	RetrainLoopExperimentOverallHistory* overall_history = (RetrainLoopExperimentOverallHistory*)run_helper.experiment_history;
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

		train_continue_target_activate(problem,
									   context,
									   inner_exit_depth,
									   inner_exit_node,
									   run_helper,
									   parent_scope_node_history);
	} else {
		train_continue_non_target_activate(problem,
										   context,
										   inner_exit_depth,
										   inner_exit_node,
										   run_helper,
										   parent_scope_node_history);
	}
}

void RetrainLoopExperiment::train_continue_target_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	RetrainLoopExperimentOverallHistory* overall_history = (RetrainLoopExperimentOverallHistory*)run_helper.experiment_history;

	int iter_index = 0;
	while (true) {
		if (iter_index >= this->scope_node->max_iters + 1) {
			break;
		}

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

		if (iter_index < this->scope_node->max_iters) {
			/**
			 * - capture continue_score without this->scope_node->continue_score_mod
			 */
			double continue_score = 0.0;

			for (int s_index = 0; s_index < (int)this->scope_node->loop_state_is_local.size(); s_index++) {
				if (this->scope_node->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						} else {
							continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						} else {
							continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						}
					}
				}
			}

			this->i_target_val_histories.push_back(continue_score);
		}

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

void RetrainLoopExperiment::train_continue_non_target_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		int& inner_exit_depth,
		AbstractNode*& inner_exit_node,
		RunHelper& run_helper,
		ScopeNodeHistory* parent_scope_node_history) {
	int iter_index = 0;
	while (true) {
		if (iter_index >= this->scope_node->max_iters) {
			break;
		}

		double continue_score = this->scope_node->continue_score_mod;
		double halt_score = this->scope_node->halt_score_mod;

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.end(); it++) {
				map<int, double>::iterator halt_weight_it = this->halt_input_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_input_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += halt_weight_it->second * normalized;
					} else {
						halt_score += halt_weight_it->second * it->second.val;
					}
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.end(); it++) {
				map<int, double>::iterator halt_weight_it = this->halt_local_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_local_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += halt_weight_it->second * normalized;
					} else {
						halt_score += halt_weight_it->second * it->second.val;
					}
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
			for (map<State*, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.begin();
					it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.end(); it++) {
				map<State*, double>::iterator halt_weight_it = this->halt_temp_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_temp_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += halt_weight_it->second * normalized;
					} else {
						halt_score += halt_weight_it->second * it->second.val;
					}
				}
			}
		}

		if (this->state == RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE_PRE) {
			for (int s_index = 0; s_index < (int)this->scope_node->loop_state_is_local.size(); s_index++) {
				if (this->scope_node->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						} else {
							continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->scope_node->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->scope_node->loop_continue_weights[s_index] * normalized;
						} else {
							continue_score += this->scope_node->loop_continue_weights[s_index] * it->second.val;
						}
					}
				}
			}
		} else {
			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
				for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.begin();
						it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].input_state_vals.end(); it++) {
					map<int, double>::iterator continue_weight_it = this->continue_input_state_weights[c_index].find(it->first);
					if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += continue_weight_it->second * normalized;
						} else {
							continue_score += continue_weight_it->second * it->second.val;
						}
					}
				}
			}

			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
				for (map<int, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.begin();
						it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].local_state_vals.end(); it++) {
					map<int, double>::iterator continue_weight_it = this->continue_local_state_weights[c_index].find(it->first);
					if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += continue_weight_it->second * normalized;
						} else {
							continue_score += continue_weight_it->second * it->second.val;
						}
					}
				}
			}

			for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
				for (map<State*, StateStatus>::iterator it = context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.begin();
						it != context[context.size() - 1 - this->scope_node->loop_scope_context.size() + c_index].temp_state_vals.end(); it++) {
					map<State*, double>::iterator continue_weight_it = this->continue_temp_state_weights[c_index].find(it->first);
					if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += continue_weight_it->second * normalized;
						} else {
							continue_score += continue_weight_it->second * it->second.val;
						}
					}
				}
			}
		}

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_halt;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_halt = false;
		} else {
			decision_is_halt = true;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_halt;
		if (abs(halt_score - continue_score) > DECISION_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
			decision_is_halt = halt_score > continue_score;
		} else {
			uniform_int_distribution<int> distribution(0, 1);
			decision_is_halt = distribution(generator);
		}
		#endif /* MDEBUG */

		if (decision_is_halt) {
			break;
		} else {
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
}

void RetrainLoopExperiment::train_continue_backprop(
		double target_val,
		RetrainLoopExperimentOverallHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		if (history->num_iters == this->scope_node->max_iters) {
			this->i_target_val_histories.push_back(target_val - this->scope_node->continue_score_mod);
		}
		for (int h_index = 0; h_index < history->num_iters; h_index++) {
			if (this->i_target_val_histories[this->i_target_val_histories.size() - history->num_iters + h_index] < target_val) {
				this->i_target_val_histories[this->i_target_val_histories.size() - history->num_iters + h_index] = target_val;
			}
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

			this->continue_input_state_weights = vector<map<int, double>>(this->scope_node->loop_scope_context.size());
			this->continue_local_state_weights = vector<map<int, double>>(this->scope_node->loop_scope_context.size());
			this->continue_temp_state_weights = vector<map<State*, double>>(this->scope_node->loop_scope_context.size());
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
							this->continue_input_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
						for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
								it != p_local_state_vals[c_index].end(); it++) {
							this->continue_local_state_weights[c_index][it->first] = weights(s_index);
							s_index++;
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
						for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
								it != p_temp_state_vals[c_index].end(); it++) {
							this->continue_temp_state_weights[c_index][it->first] = weights(s_index);
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

			new_obs_experiment(this,
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

			if (this->state == RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE_PRE) {
				this->i_scope_histories.reserve(solution->curr_num_datapoints);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints);

				this->state = RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				// this->state == RETRAIN_LOOP_EXPERIMENT_STATE_TRAIN_CONTINUE
				this->sub_state_iter++;
				if (this->sub_state_iter >= TRAIN_CONTINUE_ITERS) {
					for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
						map<int, double>::iterator continue_it = this->continue_input_state_weights[c_index].begin();
						while (continue_it != this->continue_input_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<int, double>::iterator halt_it = this->halt_input_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_input_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
								if (halt_it != this->halt_input_state_weights[c_index].end()) {
									this->halt_input_state_weights[c_index].erase(halt_it);
								}

								continue_it = this->continue_input_state_weights[c_index].erase(continue_it);
							} else {
								if (halt_it == this->halt_input_state_weights[c_index].end()) {
									this->halt_input_state_weights[c_index][continue_it->first] = 0.0;
								}

								continue_it++;
							}
						}

						map<int, double>::iterator halt_it = this->halt_input_state_weights[c_index].begin();
						while (halt_it != this->halt_input_state_weights[c_index].end()) {
							map<int, double>::iterator continue_it = this->continue_input_state_weights[c_index].find(halt_it->first);
							if (continue_it == this->continue_input_state_weights[c_index].end()) {
								double halt_weight = halt_it->second;
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
									halt_it = this->halt_input_state_weights[c_index].erase(halt_it);
								} else {
									this->continue_input_state_weights[c_index][halt_it->first] = 0.0;

									halt_it++;
								}
							} else {
								halt_it++;
							}
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
						map<int, double>::iterator continue_it = this->continue_local_state_weights[c_index].begin();
						while (continue_it != this->continue_local_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<int, double>::iterator halt_it = this->halt_local_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_local_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
								if (halt_it != this->halt_local_state_weights[c_index].end()) {
									this->halt_local_state_weights[c_index].erase(halt_it);
								}

								continue_it = this->continue_local_state_weights[c_index].erase(continue_it);
							} else {
								if (halt_it == this->halt_local_state_weights[c_index].end()) {
									this->halt_local_state_weights[c_index][continue_it->first] = 0.0;
								}

								continue_it++;
							}
						}

						map<int, double>::iterator halt_it = this->halt_local_state_weights[c_index].begin();
						while (halt_it != this->halt_local_state_weights[c_index].end()) {
							map<int, double>::iterator continue_it = this->continue_local_state_weights[c_index].find(halt_it->first);
							if (continue_it == this->continue_local_state_weights[c_index].end()) {
								double halt_weight = halt_it->second;
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
									halt_it = this->halt_local_state_weights[c_index].erase(halt_it);
								} else {
									this->continue_local_state_weights[c_index][halt_it->first] = 0.0;

									halt_it++;
								}
							} else {
								halt_it++;
							}
						}
					}

					for (int c_index = 0; c_index < (int)this->scope_node->loop_scope_context.size(); c_index++) {
						map<State*, double>::iterator continue_it = this->continue_temp_state_weights[c_index].begin();
						while (continue_it != this->continue_temp_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<State*, double>::iterator halt_it = this->halt_temp_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_temp_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
								if (halt_it != this->halt_temp_state_weights[c_index].end()) {
									this->halt_temp_state_weights[c_index].erase(halt_it);
								}

								continue_it = this->continue_temp_state_weights[c_index].erase(continue_it);
							} else {
								if (halt_it == this->halt_temp_state_weights[c_index].end()) {
									this->halt_temp_state_weights[c_index][continue_it->first] = 0.0;
								}

								continue_it++;
							}
						}

						map<State*, double>::iterator halt_it = this->halt_temp_state_weights[c_index].begin();
						while (halt_it != this->halt_temp_state_weights[c_index].end()) {
							map<State*, double>::iterator continue_it = this->continue_temp_state_weights[c_index].find(halt_it->first);
							if (continue_it == this->continue_temp_state_weights[c_index].end()) {
								double halt_weight = halt_it->second;
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * this->existing_standard_deviation) {
									halt_it = this->halt_temp_state_weights[c_index].erase(halt_it);
								} else {
									this->continue_temp_state_weights[c_index][halt_it->first] = 0.0;

									halt_it++;
								}
							} else {
								halt_it++;
							}
						}
					}

					this->state = RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE;
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
