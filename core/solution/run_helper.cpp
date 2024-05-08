#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "new_action_tracker.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->curr_depth = 0;
	this->max_depth = 0;

	this->num_decisions = 0;
	this->num_actions = 0;

	this->exceeded_limit = false;

	if (solution->state == SOLUTION_STATE_GENERALIZE) {
		this->new_action_history = new NewActionHistory();
	} else {
		this->new_action_history = NULL;
	}
}

RunHelper::~RunHelper() {
	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}

	if (this->new_action_history != NULL) {
		delete this->new_action_history;
	}
}
