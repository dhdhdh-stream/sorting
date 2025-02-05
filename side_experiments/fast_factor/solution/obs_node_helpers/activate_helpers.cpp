#include "obs_node.h"

#include "factor.h"
#include "scope.h"
#include "problem.h"

using namespace std;

void ObsNode::activate(AbstractNode*& curr_node,
					   Problem* problem,
					   RunHelper& run_helper,
					   ScopeHistory* scope_history) {
	if (this->is_used) {
		ObsNodeHistory* history = new ObsNodeHistory(this);
		scope_history->node_histories[this->id] = history;

		vector<double> obs = problem->get_observations();
		history->obs_history = obs;

		history->factor_initialized = vector<bool>(this->factors.size());
		history->factor_values = vector<double>(this->factors.size());
	}

	curr_node = this->next_node;
}
