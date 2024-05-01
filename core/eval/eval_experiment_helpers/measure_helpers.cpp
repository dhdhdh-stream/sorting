#include "eval_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

void EvalExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		EvalExperimentHistory* history) {
	vector<double> input_vals(this->decision_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->decision_input_node_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->decision_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->decision_input_node_contexts[i_index].size()-1) {
					if (it->first->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->decision_input_obs_indexes[i_index]];
					} else {
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}

	double existing_predicted_score = this->existing_average_score;
	for (int i_index = 0; i_index < (int)this->decision_input_node_contexts.size(); i_index++) {
		existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
	}
	if (this->existing_network != NULL) {
		vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
			existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
				existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
			}
		}
		this->existing_network->activate(existing_network_input_vals);
		existing_predicted_score += this->existing_network->output->acti_vals[0];
	}

	double new_predicted_score = this->new_average_score;
	for (int i_index = 0; i_index < (int)this->decision_input_node_contexts.size(); i_index++) {
		new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
	}
	if (this->new_network != NULL) {
		vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
			new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
				new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
			}
		}
		this->new_network->activate(new_network_input_vals);
		new_predicted_score += this->new_network->output->acti_vals[0];
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
	bool decision_is_branch = new_predicted_score >= existing_predicted_score;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		this->branch_count++;

		for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
			problem->perform_action(this->actions[a_index]->action);
		}

		curr_node = this->exit_next_node;
	} else {
		this->original_count++;
	}
}

void EvalExperiment::measure_back_activate(
		vector<ContextLayer>& context,
		EvalExperimentHistory* history) {
	vector<double> input_vals(this->eval_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
		int curr_layer = 0;
		ScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->eval_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->eval_input_node_contexts[i_index].size()-1) {
					if (it->first->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
						input_vals[i_index] = action_node_history->obs_snapshot[this->eval_input_obs_indexes[i_index]];
					} else {
						BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
						if (branch_node_history->is_branch) {
							input_vals[i_index] = 1.0;
						} else {
							input_vals[i_index] = -1.0;
						}
					}
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}

	double score = this->original_average_score;
	for (int i_index = 0; i_index < (int)this->eval_input_node_contexts.size(); i_index++) {
		score += input_vals[i_index] * this->eval_linear_weights[i_index];
	}
	if (this->eval_network != NULL) {
		vector<vector<double>> network_input_vals(this->eval_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->eval_network_input_indexes.size(); i_index++) {
			network_input_vals[i_index] = vector<double>(this->eval_network_input_indexes[i_index].size());
			for (int s_index = 0; s_index < (int)this->eval_network_input_indexes[i_index].size(); s_index++) {
				network_input_vals[i_index][s_index] = input_vals[this->eval_network_input_indexes[i_index][s_index]];
			}
		}
		this->eval_network->activate(network_input_vals);
		score += this->eval_network->output->acti_vals[0];
	}

	history->predicted_scores.push_back(score);
}

void EvalExperiment::measure_backprop(double target_val,
									  RunHelper& run_helper) {
	EvalExperimentHistory* history = (EvalExperimentHistory*)run_helper.experiment_histories.back();

	for (int h_index = 0; h_index < (int)history->predicted_scores.size(); h_index++) {
		this->combined_misguess += (history->predicted_scores[h_index] - target_val) * (history->predicted_scores[h_index] - target_val);

		this->state_iter++;
	}

	if (this->state_iter >= NUM_DATAPOINTS) {
		this->combined_misguess /= this->state_iter;

		double branch_weight = (double)this->branch_count / (double)(this->original_count + this->branch_count);

		if (branch_weight > PASS_THROUGH_BRANCH_WEIGHT
				&& this->new_average_score >= this->existing_average_score) {
			this->is_pass_through = true;
		} else {
			this->is_pass_through = false;
		}

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (branch_weight > 0.01
				&& this->combined_misguess < this->original_average_misguess) {
		#endif /* MDEBUG */
			this->original_average_misguess = 0.0;

			this->state = EVAL_EXPERIMENT_STATE_VERIFY_1ST_EXISTING;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
