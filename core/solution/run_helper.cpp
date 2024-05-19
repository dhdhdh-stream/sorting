#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"

using namespace std;

RunHelper::RunHelper() {
	this->num_decisions = 0;
	this->num_actions = 0;

	this->num_actions_limit = -1;

	this->experiment_scope_history = NULL;

	this->success_duplicate = NULL;
}
