#include "scope.h"

#include <iostream>

#include "globals.h"
#include "obs_network.h"
#include "score_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::experiment_start_activate(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	scope_history->obs = obs;

	this->start_obs_network->activate(wrapper->states.back(),
									  obs);

	if (this->dependencies.size() > 0) {
		scope_history->state = wrapper->states.back();
	}
}
