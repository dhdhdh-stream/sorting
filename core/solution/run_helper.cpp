#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	this->curr_depth = 0;
	this->max_depth = 0;

	this->exceeded_limit = false;

	this->experiment_history = NULL;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}
