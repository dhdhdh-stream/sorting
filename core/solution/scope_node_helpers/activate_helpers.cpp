#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "constants.h"
#include "full_network.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void ScopeNode::activate(AbstractNode*& curr_node,
						 Problem& problem,
						 vector<ContextLayer>& context,
						 int& exit_depth,
						 AbstractNode*& exit_node,
						 RunHelper& run_helper,
						 ScopeNodeHistory* history) {
	map<int, StateStatus> input_state_vals;
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		if (this->input_types[i_index] == INPUT_TYPE_STATE) {
			if (this->input_outer_is_local[i_index]) {
				StateStatus state_status;
				map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().local_state_vals.end()) {
					state_status = it->second;
				}
				input_state_vals[this->input_inner_indexes[i_index]] = state_status;
			} else {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->input_outer_indexes[i_index]);
				if (it != context.back().input_state_vals.end()) {
					input_state_vals[this->input_inner_indexes[i_index]] = it->second;
				}
			}
		} else {
			input_state_vals[this->input_inner_indexes[i_index]] = StateStatus(this->input_init_vals[i_index],
																			   this->input_init_index_vals[i_index]);
		}
	}

	context.back().node = this;

	context.push_back(ContextLayer());

	context.back().scope = this->inner_scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->inner_scope);
	history->inner_scope_history = inner_scope_history;
	context.back().scope_history = inner_scope_history;

	int inner_exit_depth = -1;
	AbstractNode* inner_exit_node = NULL;

	if (this->is_loop) {
		int iter_index = 0;
		while (true) {
			if (iter_index > this->max_iters+3) {
				run_helper.exceeded_limit = true;
				break;
			}

			double continue_score = this->continue_score_mod;
			double halt_score = this->halt_score_mod;

			for (int s_index = 0; s_index < (int)this->loop_state_is_local.size(); s_index++) {
				if (this->loop_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context[context.size()-2].local_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context[context.size()-2].input_state_vals.find(this->loop_state_indexes[s_index]);
					if (it != context[context.size()-2].input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							continue_score += this->loop_continue_weights[s_index] * normalized;
							halt_score += this->loop_halt_weights[s_index] * normalized;
						} else {
							continue_score += this->loop_continue_weights[s_index] * it->second.val;
							halt_score += this->loop_halt_weights[s_index] * it->second.val;
						}
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
				#if !defined(MDEBUG)
				/**
				 * - update even if explore
				 *   - cannot result in worst performance as would previously be -1.0 anyways
				 */
				if (iter_index > this->max_iters) {
					this->max_iters = iter_index;
				}
				#endif /* MDEBUG */
				break;
			} else {
				this->inner_scope->activate(problem,
											context,
											inner_exit_depth,
											inner_exit_node,
											run_helper,
											iter_index,
											inner_scope_history);

				for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
					map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
					if (inner_it != context.back().input_state_vals.end()) {
						if (this->output_outer_is_local[o_index]) {
							context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
						} else {
							map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
							if (outer_it != context[context.size()-2].input_state_vals.end()) {
								outer_it->second = inner_it->second;
							}
						}
					}
				}
				/**
				 * - set back after each iter for decision
				 */

				if (inner_exit_depth != -1
						|| run_helper.exceeded_limit) {
					break;
				} else {
					iter_index++;
					// continue
				}
			}
		}
	} else {
		this->inner_scope->activate(problem,
									context,
									inner_exit_depth,
									inner_exit_node,
									run_helper,
									0,
									inner_scope_history);

		for (int o_index = 0; o_index < (int)this->output_inner_indexes.size(); o_index++) {
			map<int, StateStatus>::iterator inner_it = context.back().input_state_vals.find(this->output_inner_indexes[o_index]);
			if (inner_it != context.back().input_state_vals.end()) {
				if (this->output_outer_is_local[o_index]) {
					context[context.size()-2].local_state_vals[this->output_outer_indexes[o_index]] = inner_it->second;
				} else {
					map<int, StateStatus>::iterator outer_it = context[context.size()-2].input_state_vals.find(this->output_outer_indexes[o_index]);
					if (outer_it != context[context.size()-2].input_state_vals.end()) {
						outer_it->second = inner_it->second;
					}
				}
			}
		}
		/**
		 * - intuitively, pass by reference out
		 *   - so keep even if early exit
		 * 
		 * - also will be how inner branches affect outer scopes on early exit
		 */
	}

	if (inner_scope_history->inner_pass_through_experiment != NULL) {
		inner_scope_history->inner_pass_through_experiment->parent_scope_end_activate(
			context,
			run_helper,
			inner_scope_history);
	}
	/**
	 * - triggers once even if multiple due to loops
	 *   - may lead to misguesses not aligning (even more), but hopefully won't be big impact
	 */

	context.pop_back();

	context.back().node = NULL;

	if (inner_exit_depth == -1) {
		curr_node = this->next_node;

		if (this->experiment != NULL) {
			this->experiment->activate(curr_node,
									   problem,
									   context,
									   exit_depth,
									   exit_node,
									   run_helper,
									   history->experiment_history);
		}
	} else if (inner_exit_depth == 0) {
		curr_node = inner_exit_node;
	} else {
		exit_depth = inner_exit_depth-1;
		exit_node = inner_exit_node;
	}
}
