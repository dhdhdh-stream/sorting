#include "branch_experiment.h"

using namespace std;

void BranchExperiment::wrapup_transform() {
	Scope* new_scope = new Scope(this->new_num_states,
								 this->new_states_initialized,
								 this->new_state_families,
								 this->new_default_state_classes);


	for (int a_index = 0; a_index < this->num_steps; a_index++) {
		if (this->step_types[a_index] == BRANCH_EXPERIMENT_STEP_TYPE_ACTION) {
			ActionNode* new_node = new ActionNode();
		} else {

		}
	}

	
}
