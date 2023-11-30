#include "loop_experiment.h"

#include <cmath>
#include <iostream>

#include "full_network.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void LoopExperiment::measure_activate(Problem& problem,
									  vector<ContextLayer>& context,
									  RunHelper& run_helper) {
	this->measure_num_instances++;

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

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_halt;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_halt = true;
		} else {
			decision_is_halt = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_halt = halt_score > continue_score;
		#endif /* MDEBUG */

		if (decision_is_halt) {
			break;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->potential_loop);
			this->potential_loop->activate(problem,
										   context,
										   run_helper,
										   potential_scope_node_history);
			delete potential_scope_node_history;

			if (run_helper.exceeded_limit) {
				break;
			} else {
				iter_index++;
				// continue
			}
		}
	}

	this->measure_sum_iters += iter_index;
}

void LoopExperiment::measure_backprop(double target_val) {
	this->measure_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->measure_score /= solution->curr_num_datapoints;

		cout << "Branch" << endl;
		cout << "measure" << endl;

		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement = this->measure_score - this->existing_average_score;
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		cout << "this->measure_score: " << this->measure_score << endl;
		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << "score_standard_deviation: " << score_standard_deviation << endl;
		cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		double average_num_iters = (double)this->measure_sum_iters/(double)this->measure_num_instances;

		cout << "average_num_iters: " << average_num_iters << endl;

		cout << endl;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (average_num_iters > 0.1 && score_improvement_t_score > 2.326) {
		#endif /* MDEBUG */
			this->measure_score = 0.0;
			this->measure_num_instances = 0;
			this->measure_sum_iters = 0;

			this->o_target_val_histories.reserve(solution->curr_num_datapoints);

			this->state = LOOP_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			delete this->potential_loop;
			this->potential_loop = NULL;

			for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
				delete this->new_states[s_index];
			}
			this->new_states.clear();

			this->state = LOOP_EXPERIMENT_STATE_FAIL;
		}
	}
}
