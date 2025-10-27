#include "branch_end_node.h"

#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

void BranchEndNode::step(vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	BranchEndNodeHistory* history = new BranchEndNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	wrapper->node_context.back() = this->next_node;
}
