#include "scope_node.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeNode::verify_step(vector<double>& obs,
							int& action,
							bool& is_next,
							SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->scope->nodes[0]);

	this->scope->verify_start_activate(obs,
									   wrapper);
}

void ScopeNode::verify_exit_step(vector<double>& obs,
								 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories[wrapper->scope_histories.size() - 2];

	ScopeNodeHistory* history = new ScopeNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	history->scope_history = wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();

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
