#if defined(MDEBUG) && MDEBUG

#include "info_scope_node.h"

#include <iostream>

#include "abstract_experiment.h"
#include "info_scope.h"

using namespace std;

void InfoScopeNode::verify_activate(AbstractNode*& curr_node,
									Problem* problem,
									vector<ContextLayer>& context,
									RunHelper& run_helper,
									map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	InfoScopeNodeHistory* history = new InfoScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;

	this->scope->verify_activate(problem,
								 run_helper,
								 history->scope_history,
								 history->is_positive);

	curr_node = this->next_node;
}

#endif /* MDEBUG */