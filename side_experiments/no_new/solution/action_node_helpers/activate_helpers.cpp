#include "action_node.h"

#include <iostream>

#include "action_network.h"
#include "globals.h"
#include "obs_network.h"
#include "predict_network.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::step(vector<double>& obs,
					  int& action,
					  bool& is_next,
					  SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->run_num_actions++;
}

void ActionNode::step_callback(vector<double>& obs,
							   SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	scope_history->node_histories.push_back(history);

	this->action_network->activate(wrapper->states.back());

	// temp
	Eigen::VectorXf starting_state = wrapper->states.back();

	this->obs_network->activate(wrapper->states.back(),
								obs);

	// temp
	Eigen::VectorXf state_diff = wrapper->states.back() - starting_state;
	cout << "actual: " << state_diff << endl;
	for (int i_index = 0; i_index < 10; i_index++) {
		Eigen::VectorXf temp_state = starting_state;
		this->predict_network->activate(temp_state);
		Eigen::VectorXf temp_state_diff = temp_state - starting_state;
		cout << i_index << ": " << temp_state_diff << endl;
	}

	if (!this->is_generic) {
		wrapper->node_context.back() = this->next_node;
	}
}
