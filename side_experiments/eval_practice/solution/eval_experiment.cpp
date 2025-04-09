#include "eval_experiment.h"

#include "action_node.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"

using namespace std;

EvalExperiment::EvalExperiment(Scope* scope_context) {
	this->scope_context = scope_context;

	vector<Action> actions;
	actions.push_back(Action(0));
	actions.push_back(Action(1));
	actions.push_back(Action(2));
	actions.push_back(Action(2));
	actions.push_back(Action(3));
	actions.push_back(Action(3));
	actions.push_back(Action(0));
	actions.push_back(Action(0));
	actions.push_back(Action(1));
	actions.push_back(Action(2));

	ObsNode* start_node = new ObsNode();
	start_node->parent = this->scope_context;
	start_node->id = this->scope_context->node_counter;
	this->scope_context->node_counter++;
	this->nodes.push_back(start_node);

	Input start_input;
	start_input.scope_context = {this->scope_context};
	start_input.node_context = {start_node->id};
	start_input.obs_index = 0;
	this->inputs.push_back(start_input);

	for (int a_index = 0; a_index < (int)actions.size(); a_index++) {
		ActionNode* new_action_node = new ActionNode();
		new_action_node->parent = this->scope_context;
		new_action_node->id = this->scope_context->node_counter;
		new_action_node->action = actions[a_index];
		this->scope_context->node_counter++;
		this->nodes.push_back(new_action_node);

		ObsNode* new_obs_node = new ObsNode();
		new_obs_node->parent = this->scope_context;
		new_obs_node->id = this->scope_context->node_counter;
		this->scope_context->node_counter++;
		this->nodes.push_back(new_obs_node);

		Input new_input;
		new_input.scope_context = {this->scope_context};
		new_input.node_context = {new_obs_node->id};
		new_input.obs_index = 0;
		this->inputs.push_back(new_input);
	}

	this->score_network = NULL;

	this->state = EVAL_EXPERIMENT_TRAIN;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

EvalExperiment::~EvalExperiment() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}

	if (this->score_network != NULL) {
		delete this->score_network;
	}
}
