#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::non_seed_path_activate(AbstractNode*& curr_node) {
	if (this->step_types.size() == 0) {
		curr_node = this->exit_next_node;
	} else {
		if (this->step_types[0]->type == STEP_TYPE_ACTION) {
			curr_node = this->actions[0];
		} else if (this->step_types[0]->type == STEP_TYPE_EXISTING_SCOPE) {
			curr_node = this->existing_scopes[0];
		} else {
			curr_node = this->potential_scopes[0];
		}
	}
}
