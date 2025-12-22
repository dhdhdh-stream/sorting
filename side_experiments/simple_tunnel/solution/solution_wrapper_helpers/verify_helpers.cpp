#if defined(MDEBUG) && MDEBUG

#include "solution_wrapper.h"

#include <iostream>

#include "branch_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::verify_init() {
	this->num_actions = 1;

	this->starting_run_seed = solution->verify_seeds[0];
	cout << "this->starting_run_seed: " << this->starting_run_seed << endl;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	solution->verify_seeds.erase(solution->verify_seeds.begin());

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
}

pair<bool,int> SolutionWrapper::verify_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (this->node_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				is_next = true;
				is_done = true;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->exit_step(this);
			}
		} else {
			if (this->node_context.back()->type == NODE_TYPE_BRANCH) {
				BranchNode* branch_node = (BranchNode*)this->node_context.back();
				branch_node->verify_step(obs,
										 action,
										 is_next,
										 this);
			} else {
				this->node_context.back()->step(obs,
												action,
												is_next,
												this);
			}
		}
	}

	return {is_done, action};
}

void SolutionWrapper::verify_end() {
	delete this->scope_histories[0];

	this->scope_histories.clear();
	this->node_context.clear();
}

#endif /* MDEBUG */