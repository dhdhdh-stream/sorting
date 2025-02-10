#include "branch_experiment.h"

#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

bool BranchExperiment::measure_commit_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		ScopeHistory* temp_history) {
	if (this->select_percentage == 1.0) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
				this->best_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->best_exit_next_node;

		return true;
	} else {
		double sum_vals = this->new_average_score;
		for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
			{
				double val;
				fetch_factor_helper(scope_history,
									this->new_factor_ids[f_index],
									val);
				if (val != 0.0) {
					sum_vals += this->new_factor_weights[f_index] * val;
				}
			}
			{
				double val;
				fetch_factor_helper(temp_history,
									this->new_factor_ids[f_index],
									val);
				if (val != 0.0) {
					sum_vals += this->new_factor_weights[f_index] * val;
				}
			}
		}

		bool is_branch;
		if (sum_vals >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		if (is_branch) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
					this->best_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->best_exit_next_node;

			return true;
		} else {
			return false;
		}
	}
}
