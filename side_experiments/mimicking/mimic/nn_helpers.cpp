#include "nn_helpers.h"

#include <iostream>

#include "action_network.h"
#include "constants.h"
#include "globals.h"
#include "sample.h"
#include "state_network.h"

using namespace std;

const int TRAIN_ITERS = 100000;

void train_network(vector<Sample*>& samples,
				   StateNetwork* state_network,
				   ActionNetwork* action_network) {
	double sum_errors = 0.0;

	uniform_int_distribution<int> sample_distribution(0, samples.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		if (iter_index % 1000 == 0) {
			cout << iter_index << endl;
		}

		int sample_index = sample_distribution(generator);

		vector<double> state_vals(NUM_STATES, 0.0);

		vector<StateNetworkHistory*> state_network_histories;
		vector<ActionNetworkHistory*> action_network_histories;

		{
			StateNetworkHistory* state_network_history = new StateNetworkHistory();
			state_network->activate(samples[sample_index]->obs[0],
									0,
									state_vals,
									state_network_history);
			state_network_histories.push_back(state_network_history);

			ActionNetworkHistory* action_network_history = new ActionNetworkHistory();
			action_network->activate(state_vals,
									 action_network_history);
			action_network_histories.push_back(action_network_history);

			if (iter_index % 1000 == 0) {
				cout << action_network->output->acti_vals[
					samples[sample_index]->actions[1].move + 1] << endl;
			}

			sum_errors += (1.0 - action_network->output->acti_vals[
				samples[sample_index]->actions[1].move + 1]);
		}
		for (int a_index = 1; a_index < (int)samples[sample_index]->actions.size()-1; a_index++) {
			StateNetworkHistory* state_network_history = new StateNetworkHistory();
			state_network->activate(samples[sample_index]->obs[a_index],
									samples[sample_index]->actions[a_index].move + 1,
									state_vals,
									state_network_history);
			state_network_histories.push_back(state_network_history);

			ActionNetworkHistory* action_network_history = new ActionNetworkHistory();
			action_network->activate(state_vals,
									 action_network_history);
			action_network_histories.push_back(action_network_history);

			if (iter_index % 1000 == 0) {
				cout << action_network->output->acti_vals[
					samples[sample_index]->actions[a_index+1].move + 1] << endl;
			}

			sum_errors += (1.0 - action_network->output->acti_vals[
				samples[sample_index]->actions[a_index+1].move + 1]);
		}
		{
			StateNetworkHistory* state_network_history = new StateNetworkHistory();
			state_network->activate(samples[sample_index]->obs.back(),
									samples[sample_index]->actions.back().move + 1,
									state_vals,
									state_network_history);
			state_network_histories.push_back(state_network_history);

			ActionNetworkHistory* action_network_history = new ActionNetworkHistory();
			action_network->activate(state_vals,
									 action_network_history);
			action_network_histories.push_back(action_network_history);

			if (iter_index % 1000 == 0) {
				cout << action_network->output->acti_vals[0] << endl;
			}

			sum_errors += (1.0 - action_network->output->acti_vals[0]);
		}

		vector<double> state_errors(NUM_STATES, 0.0);

		{
			action_network->backprop(0,
									 state_errors,
									 action_network_histories.back());
			delete action_network_histories.back();

			state_network->backprop(state_errors,
									state_network_histories.back());
			delete state_network_histories.back();
		}
		for (int a_index = (int)samples[sample_index]->actions.size()-2; a_index >= 0; a_index--) {
			action_network->backprop(samples[sample_index]->actions[a_index+1].move + 1,
									 state_errors,
									 action_network_histories[a_index]);
			delete action_network_histories[a_index];

			state_network->backprop(state_errors,
									state_network_histories[a_index]);
			delete state_network_histories[a_index];
		}

		if (iter_index %10 == 0) {
			action_network->update_weights();
			state_network->update_weights();
		}

		if (iter_index % 1000 == 0) {
			cout << "sum_errors: " << sum_errors << endl;
			sum_errors = 0.0;

			cout << endl;
		}
	}
}
