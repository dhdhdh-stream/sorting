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
	context.back().node = this;

	this->scope->new_action_capture_verify_activate(problem,
													context,
													run_helper);

	context.back().node = NULL;

	/**
	 * - don't bother with history
	 */

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