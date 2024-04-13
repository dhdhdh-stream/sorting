#include "pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "exit_node.h"
#include "scope_node.h"

using namespace std;

void PassThroughExperiment::root_verify_activate(
		AbstractNode*& curr_node,
		RunHelper& run_helper) {
	if (this->best_step_types.size() == 0) {
		if (this->exit_node != NULL) {
			curr_node = this->exit_node;
		} else {
			curr_node = this->best_exit_next_node;
		}
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}
