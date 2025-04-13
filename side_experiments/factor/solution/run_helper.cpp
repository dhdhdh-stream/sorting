#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->early_exit = false;

	this->num_actions = 0;

	this->experiment_history = NULL;

	this->verify_keypoints = false;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}
