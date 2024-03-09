#include "run_helper.h"

#include "abstract_experiment.h"

using namespace std;

RunHelper::RunHelper() {
	/**
	 * - initialize this->curr_depth (and this->throw_id) outside
	 */
	this->max_depth = 0;

	this->exceeded_limit = false;
}

RunHelper::~RunHelper() {
	for (int h_index = 0; h_index < (int)this->experiment_histories.size(); h_index++) {
		delete this->experiment_histories[h_index];
	}
}
