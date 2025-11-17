#include "logic_experiment.h"

#include "abstract_logic_node.h"
#include "constants.h"
#include "logic_helpers.h"
#include "network.h"

using namespace std;

void LogicExperiment::measure_activate(vector<double>& obs,
									   double target_val) {
	this->split_network->activate(obs);
	if (this->split_network->output->acti_vals[0] > 0.0) {
		double existing_predicted = logic_eval_helper(
			this->node_context,
			obs);
		double existing_misguess = (target_val - existing_predicted)
			* (target_val - existing_predicted);

		this->eval_network->activate(obs);
		double new_predicted = this->eval_network->output->acti_vals[0];
		double new_misguess = (target_val - new_predicted)
			* (target_val - new_predicted);

		this->sum_improvement += (existing_misguess - new_misguess);
		this->count++;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		this->improvement = this->sum_improvement * this->node_context->weight;

		this->state = LOGIC_EXPERIMENT_STATE_DONE;
	}
}
