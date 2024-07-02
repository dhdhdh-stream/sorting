#if defined(MDEBUG) && MDEBUG

#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "info_branch_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void SeedExperiment::capture_verify_activate(AbstractNode*& curr_node,
											 Problem* problem,
											 vector<ContextLayer>& context,
											 RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	for (int s_index = 0; s_index < this->branch_index; s_index++) {
		if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_seed_actions[s_index]->explore_activate(
				problem,
				context.back().scope_history->node_histories);
		} else {
			this->best_seed_scopes[s_index]->explore_activate(
				problem,
				context,
				run_helper,
				context.back().scope_history->node_histories);
		}
	}

	run_helper.num_decisions++;

	vector<double> new_input_vals(this->new_input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
		int curr_layer = 0;
		AbstractScopeHistory* curr_scope_history = context.back().scope_history;
		while (true) {
			map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
				this->new_input_node_contexts[i_index][curr_layer]);
			if (it == curr_scope_history->node_histories.end()) {
				break;
			} else {
				if (curr_layer == (int)this->new_input_scope_contexts[i_index].size()-1) {
					switch (it->first->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
							new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
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
					break;
				} else {
					curr_layer++;
					curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
				}
			}
		}
	}
	this->new_network->activate(new_input_vals);
	double new_predicted_score = this->new_network->output->acti_vals[0];

	this->verify_scores.push_back(new_predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	if (new_predicted_score < 0.0) {
		for (int s_index = 0; s_index < (int)this->best_back_step_types.size(); s_index++) {
			if (this->best_back_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_back_actions[s_index]->action);
			} else {
				this->best_back_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper);
			}

			if (run_helper.exceeded_limit) {
				break;
			}
		}

		curr_node = this->best_back_exit_next_node;
	} else {
		for (int s_index = this->branch_index; s_index < (int)this->best_seed_step_types.size(); s_index++) {
			if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_seed_actions[s_index]->explore_activate(
					problem,
					context.back().scope_history->node_histories);
			} else {
				this->best_seed_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper,
					context.back().scope_history->node_histories);
			}
		}

		curr_node = this->best_seed_exit_next_node;
	}
}

void SeedExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */