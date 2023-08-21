#include "scope_node.h"

using namespace std;



void ScopeNode::seed_activate(ScopeNodeHistory* seed_history,
							  ScopeNodeHistory* history) {
	run_helper.scale_factor *= this->scope_scale_mod->weight;

	Scope* inner_scope = solution->scopes[this->inner_scope_id];

	ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
	history->inner_scope_history = inner_scope_history;
	inner_scope->seed_activate(seed_history->inner_scope_history,
							   inner_scope_history);

	run_helper.scale_factor /= this->scope_scale_mod->weight;
}
