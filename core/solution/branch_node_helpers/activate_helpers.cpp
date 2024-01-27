#include "branch_node.h"

#include <cmath>
#include <iostream>

#include "clean_experiment.h"
#include "constants.h"
#include "state_network.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "retrain_branch_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void BranchNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper) {
	bool matches_context = true;
	if (this->branch_scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->branch_scope_context.size()-1; c_index++) {
			if (context[context.size()-this->branch_scope_context.size()+c_index].scope	== NULL			// OuterExperiment edge case
					|| this->branch_scope_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].scope->id
					|| context[context.size()-this->branch_scope_context.size()+c_index].node == NULL	// explore edge case
					|| this->branch_node_context[c_index] != context[context.size()-this->branch_scope_context.size()+c_index].node->id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (this->branch_is_pass_through) {
			curr_node = this->branch_next_node;
		} else {
			if (this->experiment != NULL
					&& this->experiment->type == EXPERIMENT_TYPE_RETRAIN_BRANCH) {
				RetrainBranchExperiment* retrain_branch_experiment = (RetrainBranchExperiment*)this->experiment;
				bool is_branch;
				bool is_selected = retrain_branch_experiment->activate(
					is_branch,
					problem,
					context,
					run_helper);

				if (is_selected) {
					if (is_branch) {
						curr_node = this->branch_next_node;
					} else {
						curr_node = this->original_next_node;
					}
					return;
				}
			}

			double original_score = this->original_score_mod;
			double branch_score = this->branch_score_mod;

			for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
				if (this->decision_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						StateNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
						}

						if (this->is_potential) {
							it->second.involved_local.insert(this->decision_state_indexes[s_index]);
						}

						for (set<int>::iterator index_it = it->second.involved_input.begin();
								index_it != it->second.involved_input.end(); index_it++) {
							run_helper.curr_potential_scope->used_input_states[*index_it] = true;
						}
						for (set<int>::iterator index_it = it->second.involved_local.begin();
								index_it != it->second.involved_local.end(); index_it++) {
							run_helper.curr_potential_scope->used_local_states[*index_it] = true;
						}
						for (map<PotentialScopeNode*, set<int>>::iterator potential_it = it->second.involved_output.begin();
								potential_it != it->second.involved_output.end(); potential_it++) {
							for (set<int>::iterator index_it = potential_it->second.begin();
									index_it != potential_it->second.end(); index_it++) {
								potential_it->first->used_outputs[*index_it] = true;
							}
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().input_state_vals.end()) {
						StateNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;

							if (this->is_potential) {
								it->second.involved_input.insert(this->decision_state_indexes[s_index]);
							}

							for (set<int>::iterator index_it = it->second.involved_input.begin();
									index_it != it->second.involved_input.end(); index_it++) {
								run_helper.curr_potential_scope->used_input_states[*index_it] = true;
							}
							for (set<int>::iterator index_it = it->second.involved_local.begin();
									index_it != it->second.involved_local.end(); index_it++) {
								run_helper.curr_potential_scope->used_local_states[*index_it] = true;
							}
							for (map<PotentialScopeNode*, set<int>>::iterator potential_it = it->second.involved_output.begin();
									potential_it != it->second.involved_output.end(); potential_it++) {
								for (set<int>::iterator index_it = potential_it->second.begin();
										index_it != potential_it->second.end(); index_it++) {
									potential_it->first->used_outputs[*index_it] = true;
								}
							}
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;
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
			if (abs(branch_score - original_score) > DECISION_MIN_SCORE_IMPACT * this->decision_standard_deviation) {
				decision_is_branch = branch_score > original_score;
			} else {
				uniform_int_distribution<int> distribution(0, 1);
				decision_is_branch = distribution(generator);
			}
			#endif /* MDEBUG */

			if (decision_is_branch) {
				curr_node = this->branch_next_node;

				if (this->experiment != NULL
						&& this->experiment->type == EXPERIMENT_TYPE_CLEAN
						&& this->experiment_is_branch) {
					CleanExperiment* clean_experiment = (CleanExperiment*)this->experiment;
					clean_experiment->activate(curr_node,
											   context,
											   exit_depth,
											   exit_node,
											   run_helper);
				}
			} else {
				curr_node = this->original_next_node;

				if (this->experiment != NULL
						&& this->experiment->type == EXPERIMENT_TYPE_CLEAN
						&& !this->experiment_is_branch) {
					CleanExperiment* clean_experiment = (CleanExperiment*)this->experiment;
					clean_experiment->activate(curr_node,
											   context,
											   exit_depth,
											   exit_node,
											   run_helper);
				}
			}
		}
	} else {
		curr_node = this->original_next_node;
	}
}
