#include "signal_helpers.h"

#include "globals.h"
#include "signal.h"
#include "solution.h"

using namespace std;

void set_signal_potential_inputs(Solution* solution) {
	solution->signal->potential_inputs.clear();

	uniform_int_distribution<int> x_distribution(0, 8);
	uniform_int_distribution<int> y_distribution(0, 8);
	uniform_int_distribution<int> check_is_on_distribution(0, 3);
	for (int t_index = 0; t_index < POTENTIAL_NUM_TRIES; t_index++) {
		vector<SignalInput> inputs;
		for (int i_index = 0; i_index < SIGNAL_NODE_MAX_NUM_INPUTS; i_index++) {
			SignalInput input;
			input.x_coord = x_distribution(generator);
			input.y_coord = y_distribution(generator);
			input.check_is_on = check_is_on_distribution(generator) == 0;

			bool is_match = false;
			for (int ii_index = 0; ii_index < (int)inputs.size(); ii_index++) {
				if (input == inputs[ii_index]) {
					is_match = true;
					break;
				}
			}
			if (!is_match) {
				inputs.push_back(input);
			}
		}
		solution->signal->potential_inputs.push_back(inputs);
	}

	solution->signal->potential_count = 0;
}
