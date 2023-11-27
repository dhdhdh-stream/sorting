#include "loop_experiment.h"

using namespace std;

const int NUM_SAMPLES = 100;

const int EXPLORE_ITER_LIMIT = 6;

void LoopExperiment::explore_activate(Problem& problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper) {
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

		explore_target_activate(problem,
								context,
								run_helper);
	}
}

void LoopExperiment::explore_target_activate(Problem& problem,
											 vector<ContextLayer>& context,
											 RunHelper& run_helper) {
	double starting_score = this->existing_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->starting_input_state_weights[c_index].find(it->first);
			if (weight_it != this->starting_input_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->starting_local_state_weights[c_index].find(it->first);
			if (weight_it != this->starting_local_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			map<State*, double>::iterator weight_it = this->starting_temp_state_weights[c_index].find(it->first);
			if (weight_it != this->starting_temp_state_weights[c_index].end()) {
				FullNetwork* last_network = it->second.last_network;
				if (last_network != NULL) {
					double normalized = (it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					predicted_score += weight_it->second * normalized;
				} else {
					predicted_score += weight_it->second * it->second.val;
				}
			}
		}
	}

	LoopExperimentOverallHistory* overall_history = (LoopExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->existing_predicted_score = predicted_score;

	for (int iter_index = 0; iter_index < 1 + this->state_iter; iter_index++) {
		PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->potential_loop);
		this->potential_loop->activate(problem,
									   context,
									   run_helper,
									   potential_scope_node_history);
		delete potential_scope_node_history;
	}
}

void LoopExperiment::explore_backprop(double target_val,
									  LoopExperimentOverallHistory* history) {
	if (history->has_target) {
		double score_standard_deviation = sqrt(this->existing_score_variance);
		if (target_val - history->existing_predicted_score > score_standard_deviation) {
			Scope* containing_scope = solution->scopes[this->scope_context.back()];
			this->potential_loop->scope_node_placeholder = new ScopeNode();
			this->potential_loop->scope_node_placeholder->parent = containing_scope;
			this->potential_loop->scope_node_placeholder->id = containing_scope->node_counter;
			containing_scope->node_counter++;

			this->potential_loop->scope->id = solution->scope_counter;
			solution->scope_counter++;

			this->i_scope_histories.reserve(solution->curr_num_datapoints);
			this->i_input_state_vals_histories.reserve(solution->curr_num_datapoints);
			this->i_local_state_vals_histories.reserve(solution->curr_num_datapoints);
			this->i_temp_state_vals_histories.reserve(solution->curr_num_datapoints);
			this->i_target_val_histories.reserve(solution->curr_num_datapoints);
			this->i_starting_predicted_score_histories.reserve(solution->curr_num_datapoints);

			this->state = LOOP_EXPERIMENT_STATE_TRAIN_PRE;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			this->sub_state_iter++;
			if (this->sub_state_iter >= NUM_SAMPLES) {
				this->state_iter++;
				this->sub_state_iter = 0;
				if (this->state_iter >= EXPLORE_ITER_LIMIT) {
					delete this->potential_loop;
					this->state = LOOP_EXPERIMENT_STATE_FAIL;
				}
			}
		}
	}
}
