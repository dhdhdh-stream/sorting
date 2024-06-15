#include "pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "info_scope.h"
#include "scope.h"
#include "scope_node.h"

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
		bool inner_is_positive;
		this->best_info_scope->activate(problem,
										run_helper,
										inner_is_positive);

		if ((this->best_is_negate && !inner_is_positive)
				|| (!this->best_is_negate && inner_is_positive)) {
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
