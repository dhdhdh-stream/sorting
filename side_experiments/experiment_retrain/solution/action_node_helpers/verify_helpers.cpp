#include "action_node.h"

#include <iostream>

#include "init_network.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ActionNode::verify_step(vector<double>& obs,
							 int& action,
							 bool& is_next,
							 SolutionWrapper* wrapper) {
	action = this->action;
	is_next = true;

	wrapper->num_actions++;
}

void ActionNode::verify_step_callback(vector<double>& obs,
									  SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	ActionNodeHistory* history = new ActionNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	this->obs_network->activate(wrapper->state,
								obs);

	for (int n_index = 0; n_index < (int)this->init_networks.size(); n_index++) {
		if (match_dependency_helper(wrapper,
									this->init_network_scope_contexts[n_index],
									this->init_network_node_contexts[n_index])) {
			this->init_networks[n_index]->activate(wrapper->state,
												   obs);
		}
	}

	// // temp
	// if (wrapper->starting_run_seed == 131) {
	// 	cout << "this->id: " << this->id << endl;
	// 	cout << "wrapper->state:";
	// 	for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
	// 		cout << " " << wrapper->state[s_index];
	// 	}
	// 	cout << endl;
	// 	cout << "obs:";
	// 	for (int o_index = 0; o_index < (int)obs.size(); o_index++) {
	// 		cout << " " << obs[o_index];
	// 	}
	// 	cout << endl;
	// }

	if (this->verify_states.size() > 0) {
		if (this->verify_states[0] != wrapper->state) {
			#if defined(MDEBUG) && MDEBUG
			cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
			#endif /* MDEBUG */
			wrapper->problem->print();
			cout << "this->verify_states[0]:";
			for (int s_index = 0; s_index < (int)this->verify_states[0].size(); s_index++) {
				cout << " " << this->verify_states[0][s_index];
			}
			cout << endl;
			cout << "wrapper->state:";
			for (int s_index = 0; s_index < (int)wrapper->state.size(); s_index++) {
				cout << " " << wrapper->state[s_index];
			}
			cout << endl;
			throw invalid_argument("this->verify_states[0] != wrapper->state");
		}
		this->verify_states.erase(this->verify_states.begin());
	}

	wrapper->node_context.back() = this->next_node;
}
