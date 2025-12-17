#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	history->scope_history = inner_scope_history;
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);

	inner_scope_history->pre_obs = wrapper->problem->get_observations();
}

void ScopeNode::exit_step(SolutionWrapper* wrapper) {
	wrapper->scope_histories.back()->post_obs = wrapper->problem->get_observations();

	/**
	 * - debug
	 */
	if (this->scope->signal_status != SIGNAL_STATUS_INIT) {
		cout << "this->parent->id: " << this->parent->id << endl;
		cout << "this->id: " << this->id << endl;
		cout << "pre_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << wrapper->scope_histories.back()->pre_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << wrapper->scope_histories.back()->post_obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}

		vector<double> input = wrapper->scope_histories.back()->pre_obs;
		input.insert(input.end(), wrapper->scope_histories.back()->post_obs.begin(),
			wrapper->scope_histories.back()->post_obs.end());

		this->scope->consistency_network->activate(input);
		cout << "this->scope->consistency_network->output->acti_vals[0]: " << this->scope->consistency_network->output->acti_vals[0] << endl;

		this->scope->signal_network->activate(input);
		cout << "this->scope->signal_network->output->acti_vals[0]: " << this->scope->signal_network->output->acti_vals[0] << endl;

		cout << endl;
	}

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();

	wrapper->node_context.back() = this->next_node;
}
