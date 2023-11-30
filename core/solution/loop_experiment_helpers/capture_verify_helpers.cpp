#include "loop_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void LoopExperiment::capture_verify_activate(
		Problem& problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	Problem curr_problem = problem;
	curr_problem.current_world = curr_problem.initial_world;
	curr_problem.current_pointer = 0;
	this->verify_problems[this->state_iter] = curr_problem;
	#if defined(MDEBUG) && MDEBUG
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;
	#endif /* MDEBUG */

	int iter_index = 0;
	while (true) {
		if (iter_index > TRAIN_ITER_LIMIT) {
			run_helper.exceeded_limit = true;
			break;
		}

		double continue_score = this->continue_constant;
		double halt_score = this->halt_constant;

		vector<double> factors;

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

					if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_input_state_weights[c_index].end()) {
						factors.push_back(it->second.val);
					}
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

					if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_local_state_weights[c_index].end()) {
						factors.push_back(it->second.val);
					}
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

					if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
						factors.push_back(normalized);
					}
				} else {
					continue_score += continue_weight * it->second.val;
					halt_score += halt_weight * it->second.val;

					if (continue_weight_it != this->continue_temp_state_weights[c_index].end()) {
						factors.push_back(it->second.val);
					}
				}
			}
		}

		this->verify_continue_scores.push_back(continue_score);
		this->verify_halt_scores.push_back(halt_score);
		this->verify_factors.push_back(factors);

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
			this->potential_loop->capture_verify_activate(
				problem,
				context,
				run_helper);

			if (run_helper.exceeded_limit) {
				break;
			} else {
				iter_index++;
				// continue
			}
		}
	}
}

void LoopExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->potential_loop->scope_node_placeholder->verify_key = this;
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;
		#if defined(MDEBUG) && MDEBUG
		solution->verify_seeds = this->verify_seeds;
		#endif /* MDEBUG */

		cout << "loop success" << endl;

		Scope* parent_scope = solution->scopes[this->scope_context[0]];
		parent_scope->temp_states.insert(parent_scope->temp_states.end(),
			this->new_states.begin(), this->new_states.end());
		parent_scope->temp_state_nodes.insert(parent_scope->temp_state_nodes.end(),
			this->new_state_nodes.begin(), this->new_state_nodes.end());
		parent_scope->temp_state_scope_contexts.insert(parent_scope->temp_state_scope_contexts.end(),
			this->new_state_scope_contexts.begin(), this->new_state_scope_contexts.end());
		parent_scope->temp_state_node_contexts.insert(parent_scope->temp_state_node_contexts.end(),
			this->new_state_node_contexts.begin(), this->new_state_node_contexts.end());
		parent_scope->temp_state_obs_indexes.insert(parent_scope->temp_state_obs_indexes.end(),
			this->new_state_obs_indexes.begin(), this->new_state_obs_indexes.end());
		parent_scope->temp_state_new_local_indexes.insert(parent_scope->temp_state_new_local_indexes.end(),
			this->new_states.size(), -1);

		this->new_states.clear();
		this->new_state_nodes.clear();
		this->new_state_scope_contexts.clear();
		this->new_state_node_contexts.clear();
		this->new_state_obs_indexes.clear();

		Scope* containing_scope = solution->scopes[this->scope_context.back()];

		ScopeNode* new_loop_scope_node = this->potential_loop->scope_node_placeholder;
		containing_scope->nodes[new_loop_scope_node->id] = new_loop_scope_node;

		if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

			new_loop_scope_node->next_node_id = action_node->next_node_id;
			new_loop_scope_node->next_node = action_node->next_node;

			action_node->next_node_id = new_loop_scope_node->id;
			action_node->next_node = new_loop_scope_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

			new_loop_scope_node->next_node_id = scope_node->next_node_id;
			new_loop_scope_node->next_node = scope_node->next_node;

			scope_node->next_node_id = new_loop_scope_node->id;
			scope_node->next_node = new_loop_scope_node;
		}

		map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
		map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;

		finalize_potential_scope(this->scope_context,
								 this->node_context,
								 this->potential_loop,
								 input_scope_depths_mappings,
								 output_scope_depths_mappings);
		this->potential_loop->scope_node_placeholder = NULL;
		delete this->potential_loop;
		this->potential_loop = NULL;

		new_loop_scope_node->inner_scope->is_loop = true;
		new_loop_scope_node->inner_scope->continue_score_mod = this->continue_constant;
		new_loop_scope_node->inner_scope->halt_score_mod = this->halt_constant;
		new_loop_scope_node->inner_scope->max_iters = 4;

		finalize_loop_scope_states(new_loop_scope_node,
								   this->scope_context,
								   this->node_context,
								   this->continue_input_state_weights,
								   this->continue_local_state_weights,
								   this->continue_temp_state_weights,
								   this->halt_input_state_weights,
								   this->halt_local_state_weights,
								   this->halt_temp_state_weights,
								   input_scope_depths_mappings,
								   output_scope_depths_mappings);

		new_loop_scope_node->inner_scope->verify_key = this;
		new_loop_scope_node->inner_scope->verify_continue_scores = this->verify_continue_scores;
		new_loop_scope_node->inner_scope->verify_halt_scores = this->verify_halt_scores;
		new_loop_scope_node->inner_scope->verify_factors = this->verify_factors;

		this->state = LOOP_EXPERIMENT_STATE_SUCCESS;
	}
}
