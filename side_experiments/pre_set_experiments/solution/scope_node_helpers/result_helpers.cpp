#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::result_activate(AbstractNode*& curr_node,
								Problem* problem,
								vector<ContextLayer>& context,
								RunHelper& run_helper) {
	context.back().node_id = this->id;

	this->scope->activate(problem,
						  context,
						  run_helper);

	curr_node = this->next_node;

	if (this->is_experiment) {
		run_helper.experiments_seen.push_back(this);
	}
}
