#include "obs_node.h"

#include "abstract_experiment.h"
#include "confusion.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	history->index = (int)scope_history->node_histories.size();
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size(), false);
	history->factor_values = vector<double>(this->factors.size());

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		this->experiment->activate(
			this,
			false,
			curr_node,
			problem,
			run_helper,
			scope_history);
	} else if (this->confusion != NULL) {
		this->confusion->activate(curr_node,
								  problem,
								  run_helper);
	}
}
