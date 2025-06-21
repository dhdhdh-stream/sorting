#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "explore.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::explore_init() {
	this->run_index++;

	this->num_actions = 1;
	this->num_confusion_instances = 0;

	this->has_explore = false;

	#if defined(MDEBUG) && MDEBUG
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
	this->explore_context.push_back(NULL);
}

tuple<bool,bool,int> SolutionWrapper::explore_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;
	while (!is_next) {
		if (this->node_context.back() == NULL
				&& this->explore_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				if (this->explore_context[this->explore_context.size() - 2] != NULL) {
					Explore* explore = this->explore_context[this->explore_context.size() - 2]->explore;
					explore->explore_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->explore_exit_step(this);
				}
			}
		} else if (this->explore_context.back() != NULL) {
			Explore* explore = this->explore_context.back()->explore;
			explore->explore_step(obs,
								  action,
								  is_next,
								  fetch_action,
								  this);
		} else {
			this->node_context.back()->explore_step(obs,
													action,
													is_next,
													this);
		}
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::explore_set_action(int action) {
	Explore* explore = this->explore_context.back()->explore;
	explore->set_action(action,
						this);
}

void SolutionWrapper::explore_end(double result) {
	while (true) {
		if (this->node_context.back() == NULL
				&& this->explore_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				break;
			} else {
				if (this->explore_context[this->explore_context.size() - 2] != NULL) {
					Explore* explore = this->explore_context[this->explore_context.size() - 2]->explore;
					explore->explore_exit_step(this);
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->explore_exit_step(this);
				}
			}
		} else if (this->explore_context.back() != NULL) {
			delete this->explore_context.back();
			this->explore_context.back() = NULL;
		} else {
			this->node_context.back() = NULL;
		}
	}

	this->solution->scopes[0]->back_activate(this);

	if (this->has_explore) {
		this->solution->scopes[0]->explore_scope_histories.push_back(new ScopeHistory(this->scope_histories[0]));
		this->solution->scopes[0]->explore_target_vals.push_back(result);
	} else {
		this->solution->scopes[0]->existing_scope_histories.push_back(new ScopeHistory(this->scope_histories[0]));

		create_explore(this->scope_histories[0],
					   this);
	}


	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
	this->explore_context.clear();
}
