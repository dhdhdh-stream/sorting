#include "new_info_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

bool NewInfoExperiment::root_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->is_pass_through) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
			return false;
		}

		run_helper.branch_node_ancestors.insert(this->branch_node);

		InfoBranchNodeHistory* branch_node_history = new InfoBranchNodeHistory();
		branch_node_history->index = context.back().scope_history->node_histories.size();
		context.back().scope_history->node_histories[this->branch_node] = branch_node_history;

		if (this->use_existing) {
			bool is_positive;
			Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
			solution->info_scopes[this->existing_info_scope_index]->activate(
				problem,
				context,
				run_helper,
				is_positive);

			bool is_branch;
			if (this->existing_is_negate) {
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

				return true;
			} else {
				return false;
			}
		} else {
			run_helper.num_decisions++;

			AbstractScopeHistory* scope_history;
			this->new_info_scope->explore_activate(problem,
												   context,
												   run_helper,
												   scope_history);

			vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
			for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
					this->new_input_node_contexts[i_index]);
				if (it != scope_history->node_histories.end()) {
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
				}
			}
			this->new_network->activate(new_input_vals);
			#if defined(MDEBUG) && MDEBUG
			#else
			double new_predicted_score = this->new_network->output->acti_vals[0];
			#endif /* MDEBUG */

			delete scope_history;

			#if defined(MDEBUG) && MDEBUG
			bool decision_is_branch;
			if (run_helper.curr_run_seed%2 == 0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}
			run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
			#else
			bool decision_is_branch = new_predicted_score >= 0.0;
			#endif /* MDEBUG */

			if (decision_is_branch) {
				if (this->best_step_types.size() == 0) {
					curr_node = this->best_exit_next_node;
				} else {
					if (this->best_step_types[0] == STEP_TYPE_ACTION) {
						curr_node = this->best_actions[0];
					} else {
						curr_node = this->best_scopes[0];
					}
				}

				return true;
			} else {
				return false;
			}
		}
	}
}
