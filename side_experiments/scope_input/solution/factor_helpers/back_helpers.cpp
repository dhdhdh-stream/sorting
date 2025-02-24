#include "factor.h"

#include "network.h"
#include "solution_helpers.h"

using namespace std;

double Factor::back_activate(RunHelper& run_helper,
							 ScopeHistory* scope_history) {
	// run_helper.num_analyze += (int)this->inputs.size();

	vector<double> input_vals(this->inputs.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
		fetch_input(run_helper,
					scope_history,
					this->inputs[i_index],
					input_vals[i_index]);
	}
	this->network->activate(input_vals);

	return this->network->output->acti_vals[0];
}
