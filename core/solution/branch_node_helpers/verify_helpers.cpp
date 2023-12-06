#include "branch_node.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void BranchNode::verify_activate(Problem* problem,
								 bool& is_branch,
								 vector<ContextLayer>& context,
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
			is_branch = true;
		} else {
			double original_score = this->original_score_mod;
			double branch_score = this->branch_score_mod;

			vector<double> factors;

			for (int s_index = 0; s_index < (int)this->decision_state_is_local.size(); s_index++) {
				if (this->decision_state_is_local[s_index]) {
					map<int, StateStatus>::iterator it = context.back().local_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().local_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;

							factors.push_back(normalized);
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;

							if (it->second.val != 0.0) {
								factors.push_back(it->second.val);
							}
						}
					}
				} else {
					map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->decision_state_indexes[s_index]);
					if (it != context.back().input_state_vals.end()) {
						FullNetwork* last_network = it->second.last_network;
						if (last_network != NULL) {
							double normalized = (it->second.val - last_network->ending_mean)
								/ last_network->ending_standard_deviation;
							original_score += this->decision_original_weights[s_index] * normalized;
							branch_score += this->decision_branch_weights[s_index] * normalized;

							factors.push_back(normalized);
						} else {
							original_score += this->decision_original_weights[s_index] * it->second.val;
							branch_score += this->decision_branch_weights[s_index] * it->second.val;

							if (it->second.val != 0.0) {
								factors.push_back(it->second.val);
							}
						}
					}
				}
			}

			if (this->verify_key == run_helper.verify_key) {
				// problem->print();

				// cout << "input_state_vals" << endl;
				// for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
				// 		it != context.back().input_state_vals.end(); it++) {
				// 	int state_id;
				// 	if (it->second.last_network != NULL) {
				// 		state_id = it->second.last_network->parent_state->id;
				// 	} else {
				// 		state_id = -1;
				// 	}
				// 	cout << it->second.val << " " << state_id << endl;
				// }
				// cout << "local_state_vals" << endl;
				// for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
				// 		it != context.back().local_state_vals.end(); it++) {
				// 	cout << it->second.val << endl;
				// }

				sort(factors.begin(), factors.end());
				sort(this->verify_factors[0].begin(), this->verify_factors[0].end());

				if (this->verify_original_scores[0] != original_score
						|| this->verify_branch_scores[0] != branch_score
						|| this->verify_factors[0] != factors) {
					cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

					cout << "this->verify_original_scores[0]: " << this->verify_original_scores[0] << endl;
					cout << "original_score: " << original_score << endl;

					cout << "this->verify_branch_scores[0]: " << this->verify_branch_scores[0] << endl;
					cout << "branch_score: " << branch_score << endl;

					cout << "this->verify_factors[0]" << endl;
					for (int f_index = 0; f_index < (int)this->verify_factors[0].size(); f_index++) {
						cout << f_index << ": " << this->verify_factors[0][f_index] << endl;
					}
					cout << "factors" << endl;
					for (int f_index = 0; f_index < (int)factors.size(); f_index++) {
						cout << f_index << ": " << factors[f_index] << endl;
					}

					throw invalid_argument("branch node verify fail");
				}

				this->verify_original_scores.erase(this->verify_original_scores.begin());
				this->verify_branch_scores.erase(this->verify_branch_scores.begin());
				this->verify_factors.erase(this->verify_factors.begin());
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
			bool decision_is_branch = branch_score > original_score;
			/**
			 * - fix for determinism
			 */
			#endif /* MDEBUG */

			if (decision_is_branch) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
	} else {
		is_branch = false;
	}
}
