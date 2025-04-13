#include "factor.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

double Factor::back_activate(ScopeHistory* scope_history) {
	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		fetch_input_helper(scope_history,
						   this->inputs[i_index],
						   0,
						   input_vals[i_index]);
	}
	this->network->activate(input_vals);

	return this->network->output->acti_vals[0];
}
