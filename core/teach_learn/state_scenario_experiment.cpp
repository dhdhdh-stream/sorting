#include "state_scenario_experiment.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "state_scenario.h"

using namespace std;

const int LEARN_ITERS = 40;

StateScenarioExperiment::StateScenarioExperiment(StateScenario* scenario) {
	this->scope = new Scope();

	this->scope->id = solution->scope_counter;
	solution->scope_counter++;

	this->scope->num_input_states = 0;
	this->scope->num_local_states = 0;
	this->scope->node_counter = 0;

	vector<AbstractNode*> new_nodes;

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->parent = this->scope;
	new_noop_action_node->id = this->scope->node_counter;
	this->scope->node_counter++;
	new_noop_action_node->action = Action(ACTION_NOOP);
	this->scope->nodes[new_noop_action_node->id] = new_noop_action_node;

	new_nodes.push_back(new_noop_action_node);
	this->scope->starting_node_id = new_noop_action_node->id;
	this->scope->starting_node = new_noop_action_node;

	for (int a_index = 0; a_index < (int)scenario->sequence.size(); a_index++) {
		ActionNode* new_action_node = new ActionNode();

		new_action_node->parent = this->scope;
		new_action_node->id = this->scope->node_counter;
		this->scope->node_counter++;
		this->scope->nodes[new_action_node->id] = new_action_node;

		new_action_node->action = scenario->sequence[a_index];

		new_nodes.push_back(new_action_node);
	}

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = new_nodes[n_index+1];
		}

		ActionNode* action_node = (ActionNode*)new_nodes[n_index];
		action_node->next_node_id = next_node_id;
		action_node->next_node = next_node;
	}

	this->state = STATE_SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE;
	this->state_iter = 0;
	this->sub_state_iter = 0;
}

StateScenarioExperiment::~StateScenarioExperiment() {
	// do nothing
}

void StateScenarioExperiment::activate(StateScenario* scenario,
									   RunHelper& run_helper) {
	map<int, StateStatus> input_state_vals;
	for (int s_index = 0; s_index < this->scope->num_input_states; s_index++) {
		input_state_vals[s_index] = StateStatus(0.0);
	}

	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = this->scope;
	context.back().node = NULL;

	context.back().input_state_vals = input_state_vals;

	ScopeHistory* root_history = new ScopeHistory(this->scope);
	context.back().scope_history = root_history;

	// unused
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	this->scope->activate(scenario->problem,
						  context,
						  exit_depth,
						  exit_node,
						  run_helper,
						  root_history);

	if (this->state == STATE_SCENARIO_EXPERIMENT_STATE_LEARN) {
		this->scope_histories.push_back(new ScopeHistory(root_history));

		double predicted_state = this->state_average;
		for (int s_index = 0; s_index < this->scope->num_input_states; s_index++) {
			predicted_state += context.back().input_state_vals[s_index].val;
		}
		this->predicted_state_histories.push_back(predicted_state);
	}

	delete root_history;
}

void StateScenarioExperiment::backprop(double target_state) {
	this->target_state_histories.push_back(target_state);

	if ((int)this->target_state_histories.size() >= solution->curr_num_datapoints) {
		if (this->state == STATE_SCENARIO_EXPERIMENT_STATE_LEARN_AVERAGE) {
			double sum_vals = 0.0;
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				sum_vals += this->target_state_histories[i_index];
			}
			this->state_average = sum_vals / solution->curr_num_datapoints;

			this->target_state_histories.clear();

			this->state = STATE_SCENARIO_EXPERIMENT_STATE_LEARN;
			this->state_iter = 0;
			this->sub_state_iter = 0;
		} else {
			cout << "this->sub_state_iter: " << this->sub_state_iter << endl;
			double sum_misguess = 0.0;
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				sum_misguess += abs(this->target_state_histories[i_index] - this->predicted_state_histories[i_index]);
			}
			cout << "sum_misguess: " << sum_misguess << endl;

			vector<double> obs_experiment_target_vals(solution->curr_num_datapoints);
			for (int i_index = 0; i_index < solution->curr_num_datapoints; i_index++) {
				obs_experiment_target_vals[i_index] = this->target_state_histories[i_index] - this->predicted_state_histories[i_index];
			}

			scenario_obs_experiment(this,
									this->scope_histories,
									obs_experiment_target_vals);

			for (int i_index = 0; i_index < (int)this->scope_histories.size(); i_index++) {
				delete this->scope_histories[i_index];
			}
			this->scope_histories.clear();
			this->predicted_state_histories.clear();
			this->target_state_histories.clear();

			this->sub_state_iter++;
			if (this->sub_state_iter > LEARN_ITERS) {
				solution->scopes[this->scope->id] = this->scope;

				this->state = STATE_SCENARIO_EXPERIMENT_STATE_DONE;
			}
			// else continue
		}
	}
}
