#if defined(MDEBUG) && MDEBUG

#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void CommitExperiment::capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	for (int n_index = 0; n_index < this->step_iter; n_index++) {
		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		}
	}

	double sum_vals = this->commit_new_average_score;
	for (int f_index = 0; f_index < (int)this->commit_new_factor_ids.size(); f_index++) {
		double val;
		fetch_factor_helper(scope_history,
							this->commit_new_factor_ids[f_index],
							val);
		sum_vals += this->commit_new_factor_weights[f_index] * val;
	}

	this->verify_scores.push_back(sum_vals);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	cout << "decision_is_branch: " << decision_is_branch << endl;

	if (decision_is_branch) {
		for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
			if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->save_actions[s_index]);

				run_helper.num_actions++;
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
				this->save_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}
		}

		curr_node = this->save_exit_next_node;
	} else {
		for (int n_index = this->step_iter; n_index < (int)this->new_nodes.size(); n_index++) {
			switch (this->new_nodes[n_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			}
		}

		curr_node = this->best_exit_next_node;
	}
}

void CommitExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */