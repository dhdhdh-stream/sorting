#include "scope.h"

#include <iostream>

#include "obs_network.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::start_activate(vector<double>& obs,
						   SolutionWrapper* wrapper) {
	this->start_obs_network->activate(wrapper->states.back(),
									  obs);
}
