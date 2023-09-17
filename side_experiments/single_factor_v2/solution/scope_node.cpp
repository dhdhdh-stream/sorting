#include "scope_node.h"

using namespace std;


// TODO: still cleaner to split between halfway_activate and normal activate
void ScopeNode::activate(vector<int>& starting_node_ids,
						 vector<vector<double>*>& starting_state_vals,
						 vector<double>& flat_vals,
						 vector<ContextLayer>& context,
						 int& inner_exit_depth,
						 int& inner_exit_node_id,
						 RunHelper& run_helper,
						 vector<vector<AbstractNodeHistory*>>& node_histories) {
	ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this);
	node_histories.back().push_back(scope_node_history);

	Scope* inner_scope = solution->scopes[this->inner_scope_id];


}


ScopeNodeHistory::ScopeNodeHistory(ScopeNode* node) {
	this->node = node;

	this->is_halfway = false;
}
