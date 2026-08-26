#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "init_network.h"
#include "pass_through_network.h"
#include "problem.h"
#include "scope.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::experiment_step(vector<double>& obs,
								int& action,
								bool& is_next,
								SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);

	wrapper->states.push_back(Eigen::VectorXf());
	wrapper->states.back().resize(this->scope->num_states);
	wrapper->states.back().setConstant(0.0);

	for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->in_pass_through_networks[n_index];
		double val = wrapper->states[wrapper->states.size()-2](pass_through_network->front_state_index);
		wrapper->states.back()(pass_through_network->back_state_index) += val;
	}

	this->in_network->activate(wrapper->states[wrapper->states.size()-2],
							   wrapper->states.back());

	this->scope->experiment_start_activate(obs,
										   wrapper);
}

void ScopeNode::experiment_exit_step(vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories.push_back(history);

	history->scope_history = wrapper->scope_histories.back();

	for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->out_pass_through_networks[n_index];
		double val = wrapper->states.back()[pass_through_network->front_state_index];
		wrapper->states[wrapper->states.size()-2][pass_through_network->back_state_index] += val;
	}

	this->out_network->activate(wrapper->states.back(),
								wrapper->states[wrapper->states.size()-2]);

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();

	if (!this->is_generic) {
		wrapper->node_context.back() = this->next_node;

		if (this->experiment != NULL) {
			if (this->experiment->diversity_index == -1
					|| this->experiment->diversity_index == wrapper->diversity_index) {
				this->experiment->experiment_check_activate(
					obs,
					wrapper);
			}
		}
	}
}
