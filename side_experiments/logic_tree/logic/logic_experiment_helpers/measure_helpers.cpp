#include "logic_experiment.h"

#include <iostream>

#include "abstract_logic_node.h"
#include "constants.h"
#include "logic_helpers.h"
#include "network.h"

using namespace std;

void LogicExperiment::measure_activate(vector<double>& obs,
									   double target_val) {
	// this->split_network->activate(obs);
	// if (this->split_network->output->acti_vals[0] > 0.0) {
	// temp
	if (obs[0] == 20.0) {
		double existing_predicted = logic_eval_helper(
			this->node_context,
			obs);
		double existing_misguess = (target_val - existing_predicted)
			* (target_val - existing_predicted);

		this->eval_network->activate(obs);
		double new_predicted = this->eval_network->output->acti_vals[0];
		double new_misguess = (target_val - new_predicted)
			* (target_val - new_predicted);

		// temp
		for (int x_index = 0; x_index < 5; x_index++) {
			for (int y_index = 0; y_index < 5; y_index++) {
				cout << obs[x_index * 5 + y_index] << " ";
			}
			cout << endl;
		}
		cout << "target_val: " << target_val << endl;
		cout << "existing_predicted: " << existing_predicted << endl;
		cout << "new_predicted: " << new_predicted << endl;
		cout << endl;

		this->sum_improvement += (existing_misguess - new_misguess);
		this->count++;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		// temp
		cout << "this->sum_improvement: " << this->sum_improvement << endl;
		cout << "this->count: " << this->count << endl;

		this->improvement = this->sum_improvement * this->node_context->weight;

		this->state = LOGIC_EXPERIMENT_STATE_DONE;
	}
}
