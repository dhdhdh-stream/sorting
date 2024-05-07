#include "run_helper.h"

#include "abstract_experiment.h"
#include "new_action_tracker.h"

using namespace std;

RunHelper::RunHelper() {
	this->curr_depth = 0;
	this->max_depth = 0;

	this->num_decisions = 0;
	this->num_actions = 0;

	this->exceeded_limit = false;

	this->new_action_history = NULL;
}

RunHelper::~RunHelper() {
	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}

	if (this->new_action_history != NULL) {
		delete this->new_action_history;
	}
}
