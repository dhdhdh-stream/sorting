#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "full_network.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void PassThroughExperiment::measure_new_misguess_activate(
		AbstractNode*& curr_node,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
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
			this->best_potential_scopes[s_index]->activate(problem,
														   context,
														   run_helper,
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

	double predicted_score = this->new_average_score;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		for (map<int, StateStatus>::iterator it = context[context.size() - this->scope_context.size() + c_index].input_state_vals.begin();
				it != context[context.size() - this->scope_context.size() + c_index].input_state_vals.end(); it++) {
			map<int, double>::iterator weight_it = this->new_input_state_weights[c_index].find(it->first);
			if (weight_it != this->new_input_state_weights[c_index].end()) {
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
			map<int, double>::iterator weight_it = this->new_local_state_weights[c_index].find(it->first);
			if (weight_it != this->new_local_state_weights[c_index].end()) {
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
			map<State*, double>::iterator weight_it = this->new_temp_state_weights[c_index].find(it->first);
			if (weight_it != this->new_temp_state_weights[c_index].end()) {
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

	PassThroughExperimentOverallHistory* overall_history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	overall_history->predicted_scores.push_back(predicted_score);
}

void PassThroughExperiment::measure_new_misguess_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
		this->i_misguess_histories.push_back((history->predicted_scores[i_index] - target_val) * (history->predicted_scores[i_index] - target_val));
	}

	this->state_iter++;
	if (this->state_iter >= solution->curr_num_datapoints) {
		int num_instances = (int)this->i_misguess_histories.size();

		double sum_misguess = 0.0;
		for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
			sum_misguess += this->i_misguess_histories[i_index];
		}
		this->new_average_misguess = sum_misguess / num_instances;

		this->i_misguess_histories.clear();

		double misguess_improvement = this->existing_average_misguess - this->new_average_misguess;
		double misguess_standard_deviation = sqrt(this->existing_misguess_variance);
		double misguess_improvement_t_score = misguess_improvement
			/ (misguess_standard_deviation / sqrt(solution->curr_num_datapoints));
		/**
		 * - simply using solution->curr_num_datapoints instead of tracking number of instances
		 *   - will be more conservative
		 */

		// cout << "PassThrough" << endl;
		// cout << "this->existing_average_misguess: " << this->existing_average_misguess << endl;
		// cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
		// cout << "misguess_improvement_t_score: " << misguess_improvement_t_score << endl;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (misguess_improvement_t_score > 2.326) {	// >99%
		#endif /* MDEBUG */
			// cout << "misguess success" << endl;

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					this->node_to_step_index[this->best_actions[s_index]] = s_index;
				} else {
					this->node_to_step_index[this->best_potential_scopes[s_index]->scope_node_placeholder] = s_index;
				}
			}

			uniform_int_distribution<int> distribution(0, (int)this->best_step_types.size()-1);
			this->branch_experiment_step_index = distribution(generator);

			this->branch_experiment = new BranchExperiment(
				this->scope_context,
				this->node_context);
			if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_ACTION) {
				this->branch_experiment->node_context.back() = this->best_actions[this->branch_experiment_step_index]->id;
			} else {
				this->branch_experiment->node_context.back() = this->best_potential_scopes[this->branch_experiment_step_index]->scope_node_placeholder->id;
			}
			this->branch_experiment->parent_pass_through_experiment = this;

			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
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

			this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
		}

		// cout << endl;
	}
}
