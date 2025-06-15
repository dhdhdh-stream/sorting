#include "factor.h"

#include <iostream>

#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

double Factor::back_activate(ScopeHistory* scope_history) {
	vector<double> input_vals(this->inputs.size());
	vector<bool> input_is_on(this->inputs.size());
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		double val;
		bool is_on;
		fetch_input_helper(scope_history,
						   this->inputs[i_index],
						   0,
						   val,
						   is_on);
		input_vals[i_index] = val;
		input_is_on[i_index] = is_on;
	}
	this->network->activate(input_vals,
							input_is_on);

	return this->network->output->acti_vals[0];
}
