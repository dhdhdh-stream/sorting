#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "full_network.h"
#include "globals.h"
#include "helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"
#include "utilities.h"

using namespace std;

void BranchExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	double original_predicted_score = this->existing_average_score;
	double branch_predicted_score = this->new_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_input_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_input_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_input_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_input_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].local_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].local_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<int, double>::iterator original_weight_it = this->existing_local_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_local_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<int, double>::iterator branch_weight_it = this->new_local_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_local_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
			}
		}
	}

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].temp_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].temp_state_vals.end(); it++) {
			double original_weight = 0.0;
			map<State*, double>::iterator original_weight_it = this->existing_temp_state_weights[c_index].find(it->first);
			if (original_weight_it != this->existing_temp_state_weights[c_index].end()) {
				original_weight = original_weight_it->second;
			}
			double branch_weight = 0.0;
			map<State*, double>::iterator branch_weight_it = this->new_temp_state_weights[c_index].find(it->first);
			if (branch_weight_it != this->new_temp_state_weights[c_index].end()) {
				branch_weight = branch_weight_it->second;
			}

			FullNetwork* last_network = it->second.last_network;
			if (last_network != NULL) {
				double normalized = (it->second.val - last_network->ending_mean)
					/ last_network->ending_standard_deviation;
				original_predicted_score += original_weight * normalized;
				branch_predicted_score += branch_weight * normalized;
			} else {
				original_predicted_score += original_weight * it->second.val;
				branch_predicted_score += branch_weight * it->second.val;
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
	bool decision_is_branch = branch_predicted_score > original_predicted_score;
	#endif /* MDEBUG */

	this->branch_possible++;
	if (decision_is_branch) {
		this->branch_count++;

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
				this->best_actions[s_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					action_node_history);
				delete action_node_history;
			} else {
				PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
				ScopeHistory* scope_history = new ScopeHistory(this->best_potential_scopes[s_index]->scope);
				potential_scope_node_history->scope_history = scope_history;
				this->best_potential_scopes[s_index]->activate(problem,
															   context,
															   run_helper,
															   0,
															   potential_scope_node_history);
				delete potential_scope_node_history;
			}
		}

		if (this->best_exit_depth == 0) {
			curr_node = this->best_exit_node;
		} else {
			curr_node = NULL;

			exit_depth = this->best_exit_depth-1;
			exit_node = this->best_exit_node;
		}
	}
}

void BranchExperiment::measure_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		this->combined_score /= solution->curr_num_datapoints;

		// cout << "Branch" << endl;
		// cout << "measure" << endl;
		// cout << "this->scope_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->scope_context[c_index] << endl;
		// }
		// cout << "this->node_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->node_context[c_index] << endl;
		// }
		// cout << "new explore path:";
		// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
		// 		cout << " " << this->best_actions[s_index]->action.to_string();
		// 	} else {
		// 		cout << " S";
		// 	}
		// }
		// cout << endl;

		// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
		// if (this->best_exit_node == NULL) {
		// 	cout << "this->best_exit_node_id: " << -1 << endl;
		// } else {
		// 	cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
		// }

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));

		// cout << "this->combined_score: " << this->combined_score << endl;
		// cout << "this->existing_average_score: " << this->existing_average_score << endl;
		// cout << "score_standard_deviation: " << score_standard_deviation << endl;
		// cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

		double branch_weight = (double)this->branch_count / (double)this->branch_possible;

		// cout << "branch_weight: " << branch_weight << endl;

		// cout << endl;

		if (branch_weight > 0.01 && combined_improvement_t_score > 2.326) {	// >99%
		#endif /* MDEBUG */
			this->combined_score = 0.0;
			this->branch_count = 0;
			this->branch_possible = 0;

			this->o_target_val_histories.reserve(solution->curr_num_datapoints);

			this->state = BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING;
			this->state_iter = 0;
		} else {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_potential_scopes[s_index];
				}
			}
			this->best_actions.clear();
			this->best_potential_scopes.clear();

			for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
				delete this->new_states[s_index];
			}
			this->new_states.clear();

			this->state = BRANCH_EXPERIMENT_STATE_FAIL;
		}
	}
}
