#include "solution_wrapper.h"

#include <iostream>

#include "helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

pair<bool,int> SolutionWrapper::damage_step(vector<double> obs) {
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
			if (this->node_context.back()->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)this->node_context.back();
				obs_node->damage_step(this);
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
