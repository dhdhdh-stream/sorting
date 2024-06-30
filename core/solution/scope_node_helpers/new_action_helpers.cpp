#if defined(MDEBUG) && MDEBUG

#include "scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ScopeNode::new_action_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (run_helper.scope_node_ancestors.find(this) != run_helper.scope_node_ancestors.end()) {
		run_helper.exceeded_limit = true;
		return;
	}

	context.back().node = this;
	run_helper.scope_node_ancestors.insert(this);

	ScopeHistory* scope_history = new ScopeHistory(this->scope);
	this->scope->new_action_capture_verify_activate(problem,
													context,
													run_helper,
													scope_history);

	context.back().node = NULL;
	run_helper.scope_node_ancestors.erase(this);

	/**
	 * - don't bother with history
	 */
	delete scope_history;

	curr_node = this->next_node;

	/**
	 * - still need to check experiments
	 */
	if (!run_helper.exceeded_limit) {
		for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
			bool is_selected = this->experiments[e_index]->activate(
				this,
				false,
				curr_node,
				problem,
				context,
				run_helper);
			if (is_selected) {
				return;
			}
		}
	}
}

#endif /* MDEBUG */