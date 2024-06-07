#include "info_scope_node.h"

#include "info_scope.h"

using namespace std;

void InfoScopeNode::new_action_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										map<AbstractNode*, AbstractNodeHistory*>& node_histories) {
	InfoScopeNodeHistory* history = new InfoScopeNodeHistory();
	history->index = node_histories.size();
	node_histories[this] = history;

	this->scope->activate(problem,
						  run_helper,
						  history->is_positive);

	curr_node = this->next_node;
}
