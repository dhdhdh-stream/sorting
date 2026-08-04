#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "pass_through_network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

void ScopeNode::step(vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_DAMAGE) {
		uniform_int_distribution<int> damage_distribution(0, 19);
		if (damage_distribution(generator) == 0) {
			wrapper->node_context.back() = this->next_node;
			return;
		}
	}

	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);

	wrapper->states.push_back(Eigen::VectorXf());
	wrapper->states.back().resize(this->scope->num_states);
	wrapper->states.back().setConstant(0.0);

	for (int n_index = 0; n_index < (int)this->in_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->in_pass_through_networks[n_index];
		double val = wrapper->states[wrapper->states.size()-2][pass_through_network->front_state_index];
		wrapper->states.back()[pass_through_network->back_state_index] += val;
	}

	// // temp
	// cout << "pre in wrapper->states.back():";
	// for (int s_index = 0; s_index < (int)wrapper->states.back().size(); s_index++) {
	// 	cout << " " << wrapper->states.back()[s_index];
	// }
	// cout << endl;

	this->in_network->activate(wrapper->states[wrapper->states.size()-2],
							   wrapper->states.back());

	// // temp
	// cout << "post in wrapper->states.back():";
	// for (int s_index = 0; s_index < (int)wrapper->states.back().size(); s_index++) {
	// 	cout << " " << wrapper->states.back()[s_index];
	// }
	// cout << endl;

	this->scope->start_activate(obs,
								wrapper);
}

void ScopeNode::exit_step(vector<double>& obs,
						  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->scope_history = wrapper->scope_histories.back();

	for (int n_index = 0; n_index < (int)this->out_pass_through_networks.size(); n_index++) {
		PassThroughNetwork* pass_through_network = this->out_pass_through_networks[n_index];
		double val = wrapper->states.back()[pass_through_network->front_state_index];
		wrapper->states[wrapper->states.size()-2][pass_through_network->back_state_index] += val;
	}

	// // temp
	// cout << "pre out wrapper->states[wrapper->states.size()-2]:";
	// for (int s_index = 0; s_index < (int)wrapper->states[wrapper->states.size()-2].size(); s_index++) {
	// 	cout << " " << wrapper->states[wrapper->states.size()-2][s_index];
	// }
	// cout << endl;

	this->out_network->activate(wrapper->states.back(),
								wrapper->states[wrapper->states.size()-2]);

	// // temp
	// cout << "post out wrapper->states[wrapper->states.size()-2]:";
	// for (int s_index = 0; s_index < (int)wrapper->states[wrapper->states.size()-2].size(); s_index++) {
	// 	cout << " " << wrapper->states[wrapper->states.size()-2][s_index];
	// }
	// cout << endl;

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();

	wrapper->states.pop_back();

	wrapper->node_context.back() = this->next_node;
}
