#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_analyze = 0;
	this->num_actions = 0;

	this->experiment_node = NULL;
	this->experiment_history = NULL;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}
