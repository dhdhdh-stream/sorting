#include "obs_node.h"

#include "abstract_experiment.h"
#include "factor.h"
#include "problem.h"
#include "scope.h"

using namespace std;

void ObsNode::experiment_activate(AbstractNode*& curr_node,
								  Problem* problem,
								  vector<ContextLayer>& context,
								  RunHelper& run_helper,
								  ScopeHistory* scope_history) {
	ObsNodeHistory* history = new ObsNodeHistory(this);
	scope_history->node_histories[this->id] = history;

	vector<double> obs = problem->get_observations();
	history->obs_history = obs;

	history->factor_initialized = vector<bool>(this->factors.size());
	history->factor_values = vector<double>(this->factors.size());

	curr_node = this->next_node;

	for (int e_index = 0; e_index < (int)this->experiments.size(); e_index++) {
		this->experiments[e_index]->activate(
			this,
			false,
			curr_node,
			problem,
			context,
			run_helper,
			scope_history);
	}
}
