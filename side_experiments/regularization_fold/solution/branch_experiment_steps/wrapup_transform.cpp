#include "branch_experiment.h"

using namespace std;

void BranchExperiment::wrapup_transform() {
	vector<AbstractNode*> new_nodes;
	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			ActionNode* new_node = new ActionNode();
		} else {

		}
	}

	Scope* new_scope = new Scope();
}
