#include "solution_wrapper.h"

#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void SolutionWrapper::measure_init() {
	this->num_actions = 1;

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */

	ScopeHistory* scope_history = new ScopeHistory(this->solution->scopes[0]);
	this->scope_histories.push_back(scope_history);
	this->node_context.push_back(this->solution->scopes[0]->nodes[0]);
}

tuple<bool,bool,int> SolutionWrapper::measure_step(vector<double> obs) {
	int action;
	bool is_next = false;
	bool is_done = false;
	bool fetch_action = false;

	while (!is_next) {
		bool is_signal = check_signal(obs,
									  action,
									  is_next,
									  this);
		if (!is_signal) {
			if (this->node_context.back() == NULL) {
				if (this->scope_histories.size() == 1) {
					is_next = true;
					is_done = true;
				} else {
					ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
					scope_node->exit_step(this);
				}
			} else {
				this->node_context.back()->step(obs,
												action,
												is_next,
												this);
			}
		}
	}

	return tuple<bool,bool,int>{is_done, fetch_action, action};
}

void SolutionWrapper::measure_end(double result) {
	while (true) {
		if (this->node_context.back() == NULL) {
			if (this->scope_histories.size() == 1) {
				break;
			} else {
				ScopeNode* scope_node = (ScopeNode*)this->node_context[this->node_context.size() - 2];
				scope_node->exit_step(this);
			}
		} else {
			this->node_context.back() = NULL;
		}
	}

	this->solution->existing_scope_histories.push_back(this->scope_histories[0]);
	this->solution->existing_target_val_histories.push_back(result);

	this->scope_histories.clear();
	this->node_context.clear();

	if (this->solution->existing_scope_histories.size() == MEASURE_ITERS) {
		this->solution->measure_update();
	}
}
