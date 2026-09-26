#include "scope.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "obs_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::experiment_start_activate(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	this->obs_network->activate(wrapper->states.back(),
								obs);

	if (wrapper->iters_since_update >= UPDATE_NUM_ITERS) {
		scope_history->obs = obs;
	}
}
