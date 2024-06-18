#include "solution_helpers.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void gather_possible_helper(vector<AbstractNode*>& possible_node_contexts,
							vector<int>& possible_obs_indexes,
							AbstractScopeHistory* scope_history) {
	for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->first->type) {
		case NODE_TYPE_ACTION:
		case NODE_TYPE_SCOPE:
			for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
				possible_node_contexts.push_back(it->first);
				possible_obs_indexes.push_back(o_index);
			}

			break;
		case NODE_TYPE_BRANCH:
		case NODE_TYPE_INFO_BRANCH:
			possible_node_contexts.push_back(it->first);
			possible_obs_indexes.push_back(-1);

			break;
		}
	}
}
