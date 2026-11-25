#include "scope.h"

#include <iostream>

#include "constants.h"
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

	this->obs_network->activate(wrapper->states.back(),
								obs);

	this->start_score_network->activate(wrapper->states.back());
	scope_history->start_score = this->start_score_network->output->acti_vals(0);
}

void Scope::experiment_exit_activate(SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	this->score_network->activate(wrapper->states.back());
	scope_history->score_signal = this->score_network->output->acti_vals(0);
}
