#include "action_node.h"

using namespace std;

void ActionNode::activate(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  AbstractNode*& exit_node,
						  RunHelper& run_helper,
						  vector<AbstractNodeHistory*>& node_histories) {
	ActionNodeHistory* history = new ActionNodeHistory(this);
	node_histories.push_back(history);

	problem->perform_action(this->action);
	history->obs_snapshot = problem->get_observation();

	curr_node = this->next_node;

	if (this->experiment != NULL) {
		if (this->experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperiment* branch_experiment = (BranchExperiment*)this->experiment;
			branch_experiment->activate(curr_node,
										problem,
										context,
										exit_depth,
										exit_node,
										run_helper,
										history->experiment_history);
		} else {
			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->experiment;
			pass_through_experiment->activate(curr_node,
											  problem,
											  context,
											  exit_depth,
											  exit_node,
											  run_helper,
											  history->experiment_history);
		}
	}
}
