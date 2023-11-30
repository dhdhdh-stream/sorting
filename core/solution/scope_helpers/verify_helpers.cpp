#include "scope.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "full_network.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void node_verify_activate_helper(int iter_index,
								 AbstractNode*& curr_node,
								 Problem& problem,
								 vector<ContextLayer>& context,
								 int& exit_depth,
								 AbstractNode*& exit_node,
								 RunHelper& run_helper) {
	if (curr_node->type == NODE_TYPE_ACTION) {
		ActionNode* node = (ActionNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper);
	} else if (curr_node->type == NODE_TYPE_SCOPE) {
		ScopeNode* node = (ScopeNode*)curr_node;
		node->verify_activate(curr_node,
							  problem,
							  context,
							  exit_depth,
							  exit_node,
							  run_helper);
	} else if (curr_node->type == NODE_TYPE_BRANCH) {
		BranchNode* node = (BranchNode*)curr_node;

		bool is_branch;
		node->verify_activate(problem,
							  is_branch,
							  context,
							  run_helper);

		if (is_branch) {
			curr_node = node->branch_next_node;
		} else {
			curr_node = node->original_next_node;
		}
	} else {
		ExitNode* node = (ExitNode*)curr_node;

		if (node->exit_depth == 0) {
			curr_node = node->exit_node;
		} else {
			exit_depth = node->exit_depth-1;
			exit_node = node->exit_node;
		}
	}
}

void Scope::verify_activate(Problem& problem,
							vector<ContextLayer>& context,
							int& exit_depth,
							AbstractNode*& exit_node,
							RunHelper& run_helper) {
	if (run_helper.curr_depth > run_helper.max_depth) {
		run_helper.max_depth = run_helper.curr_depth;
	}
	if (run_helper.curr_depth > solution->depth_limit) {
		run_helper.exceeded_limit = true;
		return;
	}
	run_helper.curr_depth++;

	if (this->is_loop) {
		int iter_index = 0;
		while (true) {
			if (iter_index > this->max_iters+3) {
				run_helper.exceeded_limit = true;
				break;
			}

			double continue_score = this->continue_score_mod;
			double halt_score = this->halt_score_mod;

			vector<double> factors;

			for (int s_index = 0; s_index < (int)this->loop_state_indexes.size(); s_index++) {
				map<int, StateStatus>::iterator it = context.back().input_state_vals.find(this->loop_state_indexes[s_index]);
				if (it != context.back().input_state_vals.end()) {
					FullNetwork* last_network = it->second.last_network;
					if (last_network != NULL) {
						double normalized = (it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						continue_score += this->loop_continue_weights[s_index] * normalized;
						halt_score += this->loop_halt_weights[s_index] * normalized;

						factors.push_back(normalized);
					} else {
						continue_score += this->loop_continue_weights[s_index] * it->second.val;
						halt_score += this->loop_halt_weights[s_index] * it->second.val;

						factors.push_back(it->second.val);
					}
				}
			}

			if (this->verify_key == run_helper.verify_key) {
				sort(factors.begin(), factors.end());
				sort(this->verify_factors[0].begin(), this->verify_factors[0].end());

				if (this->verify_continue_scores[0] != continue_score
						|| this->verify_halt_scores[0] != halt_score
						|| this->verify_factors[0] != factors) {
					cout << "problem index: " << NUM_VERIFY_SAMPLES - solution->verify_problems.size() << endl;

					cout << "this->verify_continue_scores[0]: " << this->verify_continue_scores[0] << endl;
					cout << "continue_score: " << continue_score << endl;

					cout << "this->verify_halt_scores[0]: " << this->verify_halt_scores[0] << endl;
					cout << "halt_score: " << halt_score << endl;

					cout << "this->verify_factors[0]" << endl;
					for (int f_index = 0; f_index < (int)this->verify_factors[0].size(); f_index++) {
						cout << f_index << ": " << this->verify_factors[0][f_index] << endl;
					}
					cout << "factors" << endl;
					for (int f_index = 0; f_index < (int)factors.size(); f_index++) {
						cout << f_index << ": " << factors[f_index] << endl;
					}

					throw invalid_argument("loop verify fail");
				}

				this->verify_continue_scores.erase(this->verify_continue_scores.begin());
				this->verify_halt_scores.erase(this->verify_halt_scores.begin());
				this->verify_factors.erase(this->verify_factors.begin());
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
				/**
				 * - update even if explore
				 *   - cannot result in worst performance as would previously be -1.0 anyways
				 */
				if (iter_index > this->max_iters) {
					this->max_iters = iter_index;
				}
				break;
			} else {
				AbstractNode* curr_node = this->starting_node;
				while (true) {
					if (exit_depth != -1
							|| curr_node == NULL
							|| run_helper.exceeded_limit) {
						break;
					}

					node_verify_activate_helper(iter_index,
												curr_node,
												problem,
												context,
												exit_depth,
												exit_node,
												run_helper);
				}

				if (exit_depth != -1
						|| run_helper.exceeded_limit) {
					break;
				} else {
					iter_index++;
					// continue
				}
			}
		}
	} else {
		AbstractNode* curr_node = this->starting_node;
		while (true) {
			if (exit_depth != -1
					|| curr_node == NULL
					|| run_helper.exceeded_limit) {
				break;
			}

			node_verify_activate_helper(0,
										curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper);
		}
	}

	run_helper.curr_depth--;
}
