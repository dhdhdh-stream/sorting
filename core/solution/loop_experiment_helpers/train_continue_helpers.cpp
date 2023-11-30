#include "loop_experiment.h"

#include <cmath>
#include <Eigen/Dense>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int TRAIN_ITERS = 2;

void LoopExperiment::train_continue_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	bool is_target = false;
	LoopExperimentOverallHistory* overall_history = (LoopExperimentOverallHistory*)run_helper.experiment_history;
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
									   run_helper,
									   history);
	} else {
		if (this->state != LOOP_EXPERIMENT_STATE_TRAIN_PRE) {
			train_continue_non_target_activate(problem,
											   context,
											   run_helper,
											   history);
		}
	}
}

void LoopExperiment::train_continue_target_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	LoopExperimentInstanceHistory* loop_experiment_history = new LoopExperimentInstanceHistory(this);
	history = loop_experiment_history;

	double start_score = this->existing_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->start_input_state_weights[c_index].find(it->first);
			if (weight_it != this->start_input_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					start_score += weight_it->second * normalized;
				} else {
					start_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->start_local_state_weights[c_index].find(it->first);
			if (weight_it != this->start_local_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					start_score += weight_it->second * normalized;
				} else {
					start_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			map<State*, double>::iterator weight_it = this->start_temp_state_weights[c_index].find(it->first);
			if (weight_it != this->start_temp_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					start_score += weight_it->second * normalized;
				} else {
					start_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	int iter_index = 0;
	while (true) {
		double halt_score = this->halt_constant;

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
				map<int, double>::iterator weight_it = this->halt_input_state_weights[c_index].find(it->first);
				if (weight_it != this->halt_input_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += weight_it->second * normalized;
					} else {
						halt_score += weight_it->second * it->second.val;
					}
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
				map<int, double>::iterator weight_it = this->halt_local_state_weights[c_index].find(it->first);
				if (weight_it != this->halt_local_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += weight_it->second * normalized;
					} else {
						halt_score += weight_it->second * it->second.val;
					}
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
				map<State*, double>::iterator weight_it = this->halt_temp_state_weights[c_index].find(it->first);
				if (weight_it != this->halt_temp_state_weights[c_index].end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						halt_score += weight_it->second * normalized;
					} else {
						halt_score += weight_it->second * it->second.val;
					}
				}
			}
		}

		LoopExperimentOverallHistory* overall_history = (LoopExperimentOverallHistory*)run_helper.experiment_history;
		overall_history->halt_predicted_scores.push_back(halt_score);

		if (iter_index >= TRAIN_ITER_LIMIT-1) {
			break;
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

		this->i_start_predicted_score_histories.push_back(start_score);

		PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->potential_loop);
		loop_experiment_history->iter_histories.push_back(potential_scope_node_history);
		this->potential_loop->activate(problem,
									   context,
									   run_helper,
									   potential_scope_node_history);

		iter_index++;
	}

	run_helper.exceeded_limit = true;
	/*
	 * - don't actually need to finish run, so simply exit
	 */
}

void LoopExperiment::train_continue_non_target_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	LoopExperimentInstanceHistory* loop_experiment_history = new LoopExperimentInstanceHistory(this);
	history = loop_experiment_history;

	int iter_index = 0;
	while (true) {
		if (iter_index > TRAIN_ITER_LIMIT) {
			run_helper.exceeded_limit = true;
			break;
		}

		double continue_score = this->continue_constant;
		double halt_score = this->halt_constant;

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<int, double>::iterator continue_weight_it = this->continue_input_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<int, double>::iterator halt_weight_it = this->halt_input_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_input_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<int, double>::iterator continue_weight_it = this->continue_local_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<int, double>::iterator halt_weight_it = this->halt_local_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_local_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
				}
			}
		}

		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
					it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
				double continue_weight = 0.0;
				map<State*, double>::iterator continue_weight_it = this->continue_temp_state_weights[c_index].find(it->first);
				if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
					continue_weight = continue_weight_it->second;
				}
				double halt_weight = 0.0;
				map<State*, double>::iterator halt_weight_it = this->halt_temp_state_weights[c_index].find(it->first);
				if (halt_weight_it != this->halt_temp_state_weights[c_index].end()) {
					halt_weight = halt_weight_it->second;
				}

				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					continue_score += continue_weight * normalized;
					halt_score += halt_weight * normalized;
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;
				}
			}
		}

		if (halt_score > continue_score) {
			break;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->potential_loop);
			loop_experiment_history->iter_histories.push_back(potential_scope_node_history);
			this->potential_loop->activate(problem,
										   context,
										   run_helper,
										   potential_scope_node_history);

			if (run_helper.exceeded_limit) {
				break;
			} else {
				iter_index++;
				// continue
			}
		}
	}
}

void LoopExperiment::train_continue_backprop(double target_val,
											 LoopExperimentOverallHistory* history) {
	this->average_instances_per_run = 0.9*this->average_instances_per_run + 0.1*history->instance_count;

	if (history->has_target) {
		vector<double> continue_target_vals(TRAIN_ITER_LIMIT-1);
		continue_target_vals[TRAIN_ITER_LIMIT-2] = history->halt_predicted_scores[TRAIN_ITER_LIMIT-1];
		for (int iter_index = TRAIN_ITER_LIMIT-3; iter_index >= 0; iter_index--) {
			if (history->halt_predicted_scores[iter_index+1] > continue_target_vals[iter_index+1]) {
				continue_target_vals[iter_index] = history->halt_predicted_scores[iter_index+1];
			} else {
				continue_target_vals[iter_index] = continue_target_vals[iter_index+1];
			}
		}

		for (int iter_index = 0; iter_index < TRAIN_ITER_LIMIT-1; iter_index++) {
			this->i_target_val_histories.push_back(continue_target_vals[iter_index]);
		}

		if ((int)this->i_target_val_histories.size() >= solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER) {
			int num_samples = this->i_target_val_histories.size();

			vector<map<int, vector<double>>> p_input_state_vals(this->scope_context.size());
			for (int i_index = 0; i_index < num_samples; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_input_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_input_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_input_state_vals[c_index].find(m_it->first);
						if (p_it == p_input_state_vals[c_index].end()) {
							p_it = p_input_state_vals[c_index].insert({m_it->first, vector<double>(num_samples, 0.0)}).first;
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
			for (int i_index = 0; i_index < num_samples; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, StateStatus>::iterator m_it = this->i_local_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_local_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<int, vector<double>>::iterator p_it = p_local_state_vals[c_index].find(m_it->first);
						if (p_it == p_local_state_vals[c_index].end()) {
							p_it = p_local_state_vals[c_index].insert({m_it->first, vector<double>(num_samples, 0.0)}).first;
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
			for (int i_index = 0; i_index < num_samples; i_index++) {
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][c_index].begin();
							m_it != this->i_temp_state_vals_histories[i_index][c_index].end(); m_it++) {
						map<State*, vector<double>>::iterator p_it = p_temp_state_vals[c_index].find(m_it->first);
						if (p_it == p_temp_state_vals[c_index].end()) {
							p_it = p_temp_state_vals[c_index].insert({m_it->first, vector<double>(num_samples, 0.0)}).first;
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
			stride_size += 1;	// for constant

			Eigen::MatrixXd inputs(num_samples, stride_size);
			for (int i_index = 0; i_index < num_samples; i_index++) {
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

				inputs(i_index, s_index) = 1.0;		// for constant
			}

			Eigen::VectorXd outputs(num_samples);
			for (int i_index = 0; i_index < num_samples; i_index++) {
				outputs(i_index) = this->i_target_val_histories[i_index]
					- this->i_start_predicted_score_histories[i_index];
			}

			this->continue_input_state_weights = vector<map<int, double>>(this->scope_context.size());
			this->continue_local_state_weights = vector<map<int, double>>(this->scope_context.size());
			this->continue_temp_state_weights = vector<map<State*, double>>(this->scope_context.size());
			Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			{
				int s_index = 0;

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_input_state_vals[c_index].begin();
							it != p_input_state_vals[c_index].end(); it++) {
						this->continue_input_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<int, vector<double>>::iterator it = p_local_state_vals[c_index].begin();
							it != p_local_state_vals[c_index].end(); it++) {
						this->continue_local_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					for (map<State*, vector<double>>::iterator it = p_temp_state_vals[c_index].begin();
							it != p_temp_state_vals[c_index].end(); it++) {
						this->continue_temp_state_weights[c_index][it->first] = weights(s_index);
						s_index++;
					}
				}

				this->continue_constant = weights(s_index);
			}

			Eigen::VectorXd predicted_scores = inputs * weights;
			Eigen::VectorXd diffs = outputs - predicted_scores;
			vector<double> obs_experiment_target_vals(num_samples);
			for (int i_index = 0; i_index < num_samples; i_index++) {
				obs_experiment_target_vals[i_index] = diffs(i_index);
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
			this->i_start_predicted_score_histories.clear();

			if (this->state == LOOP_EXPERIMENT_STATE_TRAIN_PRE) {
				this->i_scope_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				this->i_target_val_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				this->i_start_predicted_score_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);

				this->state = LOOP_EXPERIMENT_STATE_TRAIN;
				this->sub_state = LOOP_EXPERIMENT_SUB_STATE_TRAIN_HALT;
				this->state_iter = 0;
				this->sub_state_iter = 0;
			} else {
				// this->state == LOOP_EXPERIMENT_STATE_TRAIN
				this->state_iter++;
				if (this->state_iter >= TRAIN_ITERS) {
					double score_standard_deviation = sqrt(this->existing_score_variance);

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<int, double>::iterator continue_it = this->continue_input_state_weights[c_index].begin();
						while (continue_it != this->continue_input_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<int, double>::iterator halt_it = this->halt_input_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_input_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<int, double>::iterator continue_it = this->continue_local_state_weights[c_index].begin();
						while (continue_it != this->continue_local_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<int, double>::iterator halt_it = this->halt_local_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_local_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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

					for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
						map<State*, double>::iterator continue_it = this->continue_temp_state_weights[c_index].begin();
						while (continue_it != this->continue_temp_state_weights[c_index].end()) {
							double continue_weight = continue_it->second;

							double halt_weight = 0.0;
							map<State*, double>::iterator halt_it = this->halt_temp_state_weights[c_index].find(continue_it->first);
							if (halt_it != this->halt_temp_state_weights[c_index].end()) {
								halt_weight = halt_it->second;
							}

							if (abs(continue_weight - halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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
								if (abs(halt_weight) < WEIGHT_MIN_SCORE_IMPACT * score_standard_deviation) {
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

					this->state = LOOP_EXPERIMENT_STATE_MEASURE;
					this->state_iter = 0;
				} else {
					this->i_scope_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
					this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
					this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
					this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
					this->i_target_val_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
					this->i_start_predicted_score_histories.reserve(solution->curr_num_datapoints*NUM_SAMPLES_MULTIPLIER);
				}
			}
		}
	}
}
