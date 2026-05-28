#include "experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "network.h"
#include "obs_node.h"
#include "predict_run.h"
#include "utilities.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

double predict_helper(AbstractNode* node_context,
					  bool is_branch,
					  vector<double>& starting_state,
					  Wrapper* wrapper) {
	#if defined(MDEBUG) && MDEBUG
	wrapper->run_index++;
	wrapper->starting_run_seed = wrapper->run_index;
	wrapper->curr_run_seed = xorshift(wrapper->starting_run_seed);
	#endif /* MDEBUG */

	PredictRun* run = new PredictRun();
	run->wrapper = wrapper;
	switch (node_context->type) {
	case NODE_TYPE_OBS:
		{
			ObsNode* obs_node = (ObsNode*)node_context;
			run->node_context = obs_node->next_node;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)node_context;
			run->node_context = action_node->next_node;
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)node_context;
			if (is_branch) {
				run->node_context = branch_node->branch_next_node;
			} else {
				run->node_context = branch_node->original_next_node;
			}
		}
		break;
	}
	run->state = starting_state;

	while (true) {
		if (run->node_context == NULL
				&& run->experiment_context == NULL) {
			break;
		}

		if (run->experiment_context != NULL) {
			Experiment* experiment = run->experiment_context->experiment;
			experiment->predict_step(run);
		} else {
			run->node_context->predict_step(run);
		}
	}

	WorldModel* world_model = wrapper->world_model;

	double sum_score = 0.0;
	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(run->state[world_model->final_network_inputs[n_index][i_index]]);
		}
		world_model->final_networks[n_index]->activate(inputs);
		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	}

	delete run;

	return sum_score;
}

/**
 * - predict explore
 */
double predict_helper(vector<int>& actions,
					  AbstractNode* exit_next_node,
					  vector<double>& starting_state,
					  Wrapper* wrapper) {
	#if defined(MDEBUG) && MDEBUG
	wrapper->run_index++;
	wrapper->starting_run_seed = wrapper->run_index;
	wrapper->curr_run_seed = xorshift(wrapper->starting_run_seed);
	#endif /* MDEBUG */

	PredictRun* run = new PredictRun();
	run->wrapper = wrapper;
	run->node_context = exit_next_node;
	run->experiment_context = NULL;
	run->state = starting_state;

	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		action_helper(actions[a_index],
					  run->state,
					  run->wrapper);
	}

	while (true) {
		if (run->node_context == NULL
				&& run->experiment_context == NULL) {
			break;
		}

		if (run->experiment_context != NULL) {
			Experiment* experiment = run->experiment_context->experiment;
			experiment->predict_step(run);
		} else {
			run->node_context->predict_step(run);
		}
	}

	WorldModel* world_model = wrapper->world_model;

	double sum_score = 0.0;
	for (int n_index = 0; n_index < (int)world_model->final_networks.size(); n_index++) {
		vector<double> inputs;
		for (int i_index = 0; i_index < (int)world_model->final_network_inputs[n_index].size(); i_index++) {
			inputs.push_back(run->state[world_model->final_network_inputs[n_index][i_index]]);
		}
		world_model->final_networks[n_index]->activate(inputs);
		sum_score += world_model->final_networks[n_index]->output->acti_vals[0];
	}

	delete run;

	return sum_score;
}
