#include "solution_wrapper.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "globals.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::init(int run_type,
						   vector<double> obs) {
	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	this->run_type = run_type;

	this->state.resize(this->solution->num_states);
	this->state.setConstant(0.0);

	this->num_actions = 1;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);

	this->solution->starting_scope->start_activate(obs,
												   this);
}

pair<bool,int> SolutionWrapper::step(vector<double> obs) {
	if (this->last_was_damage) {
		// // temp
		// cout << "generic_obs_network" << endl;
		// cout << "starting state:";
		// for (int s_index = 0; s_index < (int)this->state.size(); s_index++) {
		// 	cout << " " << this->state(s_index);
		// }
		// cout << endl;

		ObsNetwork* obs_network = this->solution->generic_obs_network;
		obs_network->activate(this->state,
							  obs);

		// // temp
		// cout << "ending state:";
		// for (int s_index = 0; s_index < (int)this->state.size(); s_index++) {
		// 	cout << " " << this->state(s_index);
		// }
		// cout << endl;

		this->last_was_damage = false;
	} else {
		if (this->node_context.back() != NULL
				&& this->node_context.back()->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->node_context.back();
			action_node->step_callback(obs,
									   this);
		}
	}

	if (this->run_type == RUN_TYPE_DAMAGE) {
		uniform_int_distribution<int> damage_distribution(0, 19);
		if (damage_distribution(generator) == 0) {
			uniform_int_distribution<int> action_distribution(0, this->solution->generic_action_networks.size()-1);
			int action = action_distribution(generator);

			// // temp
			// cout << "generic_action_network" << endl;
			// cout << "action: " << action << endl;
			// cout << "starting state:";
			// for (int s_index = 0; s_index < (int)this->state.size(); s_index++) {
			// 	cout << " " << this->state(s_index);
			// }
			// cout << endl;

			ActionNetwork* action_network = this->solution->generic_action_networks[action];
			action_network->activate(this->state);

			// // temp
			// cout << "ending state:";
			// for (int s_index = 0; s_index < (int)this->state.size(); s_index++) {
			// 	cout << " " << this->state(s_index);
			// }
			// cout << endl;

			this->last_was_damage = true;

			return {false, action};
		}
	}

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL) {
			// // temp
			// Scope* scope = this->scope_histories.back()->scope;
			// scope->end_score_network->activate(this->state,
			// 								   0.0);
			// cout << "scope->end_score_network->output->acti_vals(0): " << scope->end_score_network->output->acti_vals(0) << endl;

			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->exit_step(obs,
									  this);
			}
		} else {
			this->node_context.back()->step(obs,
											action,
											is_next,
											this);
		}
	}

	return {is_done, action};
}

void SolutionWrapper::end() {
	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
}
