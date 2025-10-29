#include "branch_end_node.h"

#include <iostream>

#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void BranchEndNode::step(vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchEndNodeHistory* history = new BranchEndNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	/**
	 * - debug
	 */
	if (this->signal_network != NULL) {
		cout << "this->parent->id: " << this->parent->id << endl;
		cout << "this->id: " << this->id << endl;
		cout << "pre_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << wrapper->branch_node_stack_obs.back()[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}
		cout << "post_obs:" << endl;
		for (int i_index = 0; i_index < 5; i_index++) {
			for (int j_index = 0; j_index < 5; j_index++) {
				cout << obs[5 * i_index + j_index] << " ";
			}
			cout << endl;
		}

		vector<double> input = wrapper->branch_node_stack_obs.back();
		input.insert(input.end(), obs.begin(), obs.end());
		this->signal_network->activate(input);
		cout << "this->signal_network->output->acti_vals[0]: " << this->signal_network->output->acti_vals[0] << endl;

		cout << endl;
	}
	wrapper->branch_node_stack.pop_back();
	wrapper->branch_node_stack_obs.pop_back();

	wrapper->node_context.back() = this->next_node;
}
