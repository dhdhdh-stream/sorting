#include "branch_experiment.h"

using namespace std;

void BranchExperiment::activate(AbstractNode* experiment_node,
								bool is_branch,
								AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history) {
	BranchExperimentOverallHistory* overall_history;
	map<AbstractExperiment*, AbstractExperimentOverallHistory*>::iterator it
		= run_helper.overall_histories.find(this);
	if (it == run_helper.overall_histories.end()) {
		overall_history = new BranchExperimentOverallHistory(this);
		run_helper.overall_histories[this] = overall_history;
	} else {
		overall_history = (BranchExperimentOverallHistory*)it->second;
	}

	if (overall_history->is_active) {
		switch (this->state) {
		case BRANCH_EXPERIMENT_STATE_EXISTING_GATHER:
			existing_gather_activate(scope_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING:
			train_existing_activate(scope_history,
									overall_history);
			break;
		case BRANCH_EXPERIMENT_STATE_EXPLORE:
			explore_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 overall_history);
			break;
		case BRANCH_EXPERIMENT_STATE_NEW_GATHER:
			new_gather_activate(scope_history,
								overall_history);
			break;
		case BRANCH_EXPERIMENT_STATE_TRAIN_NEW:
			train_new_activate(curr_node,
							   problem,
							   run_helper,
							   scope_history,
							   overall_history);
			break;
		case BRANCH_EXPERIMENT_STATE_MEASURE:
			measure_activate(curr_node,
							 problem,
							 run_helper,
							 scope_history,
							 overall_history);
			break;
		}
	}
}

void BranchExperiment::backprop(AbstractExperimentInstanceHistory* instance_history,
								double target_val) {

}

void BranchExperiment::update(AbstractExperimentOverallHistory* overall_history,
							  double target_val) {

}
