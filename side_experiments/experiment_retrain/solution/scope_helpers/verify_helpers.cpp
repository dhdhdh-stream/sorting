#include "scope.h"

#include <iostream>

#include "init_network.h"
#include "negate_network.h"
#include "obs_network.h"
#include "solution_wrapper.h"

using namespace std;

void Scope::verify_start_activate(vector<double>& obs,
								  SolutionWrapper* wrapper) {
	for (int n_index = 0; n_index < (int)this->start_negate_networks.size(); n_index++) {
		this->start_negate_networks[n_index]->activate(wrapper->state);
	}

	this->start_obs_network->activate(wrapper->state,
									  obs);

	for (int n_index = 0; n_index < (int)this->start_init_networks.size(); n_index++) {
		this->start_init_networks[n_index]->activate(wrapper->state,
													 obs);
	}

	// temp
	if (wrapper->starting_run_seed == 131) {
		cout << "this->id: -1" << endl;
		cout << "wrapper->state:";
		for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
			cout << " " << wrapper->state[s_index];
		}
		cout << endl;
		cout << "obs:";
		for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
			cout << " " << obs[o_index];
		}
		cout << endl;
	}
}
