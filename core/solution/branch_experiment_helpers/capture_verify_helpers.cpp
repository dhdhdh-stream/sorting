#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper) {
	run_helper.num_decisions++;

	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->new_input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			switch (it->first->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					new_input_vals[i_index] = scope_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					new_input_vals[i_index] = branch_node_history->score;
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
					if (info_branch_node_history->is_branch) {
						new_input_vals[i_index] = 1.0;
					} else {
						new_input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}
	this->new_network->activate(new_input_vals);
	double new_predicted_score = this->new_network->output->acti_vals[0];

	this->verify_scores.push_back(new_predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	cout << "context scope" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].scope->id << endl;
	}
	cout << "context node" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].node->id << endl;
	}

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	cout << "decision_is_branch: " << decision_is_branch << endl;

	if (decision_is_branch) {
		if (this->best_info_scope == NULL) {
			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}
		} else {
			bool is_positive;
			this->best_info_scope->activate(problem,
											context,
											run_helper,
											is_positive);

			bool is_branch;
			if (this->best_is_negate) {
				if (is_positive) {
					is_branch = false;
				} else {
					is_branch = true;
				}
			} else {
				if (is_positive) {
					is_branch = true;
				} else {
					is_branch = false;
				}
			}

			if (is_branch) {
				if (this->best_step_types.size() == 0) {
					curr_node = this->best_exit_next_node;
				} else {
					if (this->best_step_types[0] == STEP_TYPE_ACTION) {
						curr_node = this->best_actions[0];
					} else {
						curr_node = this->best_scopes[0];
					}
				}
			}
		}

		return true;
	} else {
		return false;
	}
}

void BranchExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		if (this->parent_experiment == NULL) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			vector<AbstractExperiment*> verify_experiments;
			verify_experiments.insert(verify_experiments.begin(), this);
			AbstractExperiment* curr_experiment = this->parent_experiment;
			while (true) {
				if (curr_experiment->parent_experiment == NULL) {
					/**
					 * - don't include root
					 */
					break;
				} else {
					verify_experiments.insert(verify_experiments.begin(), curr_experiment);
					curr_experiment = curr_experiment->parent_experiment;
				}
			}

			this->root_experiment->verify_experiments = verify_experiments;

			this->root_experiment->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

			this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_EXISTING;

			this->state = BRANCH_EXPERIMENT_STATE_ROOT_VERIFY;
		}
	}
}

#endif /* MDEBUG */