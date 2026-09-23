#include "solution_helpers.h"

#include <Eigen/Dense>

#include "action_node.h"
#include "constants.h"
#include "obs_network.h"
#include "scope.h"
#include "scope_node.h"
#include "transition_network.h"

using namespace std;

void calc_curr_state_helper(ScopeHistory* scope_history,
							Eigen::VectorXf& state) {
	Scope* scope = scope_history->scope;
	scope->obs_network->activate(state,
								 scope_history->obs);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNode* node = scope_history->node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node;
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[h_index];
				action_node->obs_network->activate(state,
												   action_node_history->obs);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node;
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[h_index];

				Eigen::VectorXf inner_state;
				inner_state.resize(NUM_STATES);
				inner_state.setConstant(0.0);

				scope_node->in_network->activate(state,
												 inner_state);

				calc_curr_state_helper(scope_node_history->scope_history,
									   inner_state);

				scope_node->out_network->activate(inner_state,
												  state);
			}
			break;
		}
	}
}
