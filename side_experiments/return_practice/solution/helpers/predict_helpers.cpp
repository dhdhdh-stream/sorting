#include "solution_helpers.h"

#include "abstract_node.h"
#include "predict_run.h"
#include "state_network.h"
#include "world_model.h"
#include "wrapper.h"

using namespace std;

double predict_helper(vector<double>& state,
					  AbstractNode* next_node,
					  Wrapper* wrapper) {
	PredictRun* run = new PredictRun();

	run->wrapper = wrapper;

	run->node_context = next_node;

	run->state = state;

	while (run->node_context != NULL) {
		run->node_context->predict_step(run);
	}

	wrapper->world_model->curr_final_network->activate(run->state);
	double predicted = wrapper->world_model->curr_final_network->output->acti_vals[0];

	delete run;

	return predicted;
}
