#include "run_helper.h"

#include "abstract_experiment.h"
#include "globals.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

RunHelper::RunHelper() {
	uniform_int_distribution<int> has_random_distribution(0, 4);
	this->can_random = has_random_distribution(generator) == 0;

	this->random_distribution = uniform_int_distribution<int>(0, 39);

	this->num_analyze = 0;
	this->num_actions = 0;

	this->experiment_history = NULL;
}

RunHelper::~RunHelper() {
	if (this->experiment_history != NULL) {
		delete this->experiment_history;
	}
}

bool RunHelper::is_random() {
	#if defined(MDEBUG) && MDEBUG
	bool result;
	if (this->can_random) {
		if (this->curr_run_seed % 5 == 0) {
			result = true;
		} else {
			result = false;
		}
		this->curr_run_seed = xorshift(this->curr_run_seed);
	} else {
		result = false;
	}
	return result;
	#else
	if (this->can_random) {
		return this->random_distribution(generator) == 0;
	} else {
		return false;
	}
	#endif /* MDEBUG */
}
