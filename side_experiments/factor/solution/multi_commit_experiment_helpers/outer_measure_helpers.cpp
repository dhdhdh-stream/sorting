#include "multi_commit_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void MultiCommitExperiment::outer_measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		MultiCommitExperimentHistory* history) {
	if (history->is_active) {
		run_helper.has_explore = true;

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

		run_helper.num_actions++;

		double sum_vals = this->commit_new_average_score;
		for (int f_index = 0; f_index < (int)this->commit_new_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(run_helper,
								scope_history,
								this->commit_new_factor_ids[f_index],
								val);
			sum_vals += this->commit_new_factor_weights[f_index] * val;
		}

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (sum_vals >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */

		if (is_branch) {
			for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
				if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->save_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
					this->save_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
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
}
