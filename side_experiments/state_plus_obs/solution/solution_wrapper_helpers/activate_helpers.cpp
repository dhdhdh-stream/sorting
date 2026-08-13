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

	this->states.push_back(Eigen::VectorXf());
	this->states.back().resize(this->solution->starting_scope->num_states);
	this->states.back().setConstant(0.0);

	this->run_num_actions = 0;

	ScopeHistory* scope_history = new ScopeHistory(this->solution->starting_scope);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->starting_scope->nodes[0]);

	this->solution->starting_scope->start_activate(obs,
												   this);
}

pair<bool,int> SolutionWrapper::step(vector<double> obs) {
	if (this->node_context.back() != NULL
			&& this->node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)this->node_context.back();
		action_node->step_callback(obs,
								   this);
	}

	int action;
	bool is_next = false;
	bool is_done = false;

	if (this->run_type == RUN_TYPE_DAMAGE) {
		uniform_int_distribution<int> damage_distribution(0, 19);
		if (damage_distribution(generator) == 0) {
			Scope* scope = this->scope_histories.back()->scope;

			uniform_int_distribution<int> distribution(0, scope->generic_action_nodes.size()-1);
			scope->generic_action_nodes[distribution(generator)]->step(
				obs,
				action,
				is_next,
				this);
		}
	}

	while (!is_next) {
		if (this->node_context.back() == NULL) {
			// // temp
			// Scope* scope = this->scope_histories.back()->scope;
			// scope->end_score_network->activate(this->states.back(),
			// 								   obs);
			// double signal = scope->end_score_network->output->acti_vals(0);
			// cout << "this->states.back():";
			// for (int s_index = 0; s_index < (int)this->states.back().size(); s_index++) {
			// 	cout << " " << this->states.back()[s_index];
			// }
			// cout << endl;
			// cout << "signal: " << signal << endl;

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

	this->states.clear();
}
