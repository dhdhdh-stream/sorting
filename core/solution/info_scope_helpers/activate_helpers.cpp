#include "info_scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "utilities.h"

using namespace std;

void InfoScope::activate(Problem* problem,
						 RunHelper& run_helper,
						 double& inner_score) {
	// if (this->experiment != NULL) {
	// 	InfoPassThroughExperiment* info_pass_through_experiment = (InfoPassThroughExperiment*)this->experiment;
	// 	info_pass_through_experiment->info_scope_activate(run_helper);
	// }

	ScopeHistory* scope_history;
	this->subscope->info_activate(problem,
								  run_helper,
								  scope_history);

	run_helper.num_decisions++;

	// if (this->experiment != NULL) {
	// 	if (run_helper.experiment_histories.size() == 1
	// 			&& run_helper.experiment_histories[0]->experiment == this->experiment) {
	// 		InfoPassThroughExperiment* info_pass_through_experiment = (InfoPassThroughExperiment*)this->experiment;
	// 		bool is_selected = info_pass_through_experiment->back_activate(
	// 			problem,
	// 			subscope_history,
	// 			result_is_positive,
	// 			run_helper);
	// 		if (is_selected) {
	// 			return;
	// 		}
	// 	}
	// }

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != scope_history->node_histories.end()) {
			switch (this->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					input_vals[i_index] = info_scope_node_history->score;
				}
				break;
			}
		}
	}
	this->network->activate(input_vals);
	inner_score = this->network->output->acti_vals[0];

	delete scope_history;
}
