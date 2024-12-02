#include "scope_node.h"

#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void ScopeNode::set_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 vector<ContextLayer>& context,
							 RunHelper& run_helper,
							 bool& experiment_seen,
							 map<pair<AbstractNode*,bool>, int>& nodes_seen) {
	context.back().node_id = this->id;

	this->scope->set_activate(problem,
							  context,
							  run_helper,
							  experiment_seen,
							  nodes_seen);

	curr_node = this->next_node;

	if (this->is_experiment) {
		experiment_seen = true;
	}

	if (!experiment_seen) {
		map<pair<AbstractNode*,bool>, int>::iterator it = nodes_seen.find({this, false});
		if (it == nodes_seen.end()) {
			nodes_seen[{this, false}] = 1;
		} else {
			it->second++;
		}
	}
}
