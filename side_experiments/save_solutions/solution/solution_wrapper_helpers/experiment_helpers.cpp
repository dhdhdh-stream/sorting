#include "solution_wrapper.h"

#include <iostream>

#include "branch_experiment.h"
#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::experiment_init() {
	if (this->curr_signal_experiment != NULL) {
		signal_experiment_init();
	} else if (this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		measure_init();
	} else {
		explore_init();
	}
}

tuple<bool,bool,int> SolutionWrapper::experiment_step(vector<double> obs) {
	if (this->curr_signal_experiment != NULL) {
		return signal_experiment_step(obs);
	} else if (this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		return measure_step(obs);
	} else {
		return explore_step(obs);
	}
}

void SolutionWrapper::set_action(int action) {
	AbstractExperiment* experiment = this->experiment_context.back()->experiment;
	experiment->set_action(action,
						   this);
}

void SolutionWrapper::experiment_end(double result) {
	if (this->curr_signal_experiment != NULL) {
		signal_experiment_end(result);
	} else if (this->solution->existing_scope_histories.size() < MEASURE_ITERS) {
		experiment_end(result);
	} else {
		explore_end(result);
	}
}
