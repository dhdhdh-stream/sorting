#include "pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

void PassThroughExperiment::root_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
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
		double inner_score;
		this->best_info_scope->activate(problem,
										run_helper,
										inner_score);

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (this->best_is_negate) {
			if (inner_score >= 0.0) {
				is_branch = false;
			} else {
				is_branch = true;
			}
		} else {
			if (inner_score >= 0.0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
		}
		#endif /* MDEBUG */

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
}
