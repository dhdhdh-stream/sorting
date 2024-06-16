#include "info_scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope_node.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void node_activate_helper(AbstractNode*& curr_node,
						  Problem* problem,
						  vector<ContextLayer>& context,
						  RunHelper& run_helper,
						  InfoScopeHistory* history) {
	switch (curr_node->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* node = (ActionNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	case NODE_TYPE_INFO_SCOPE:
		{
			InfoScopeNode* node = (InfoScopeNode*)curr_node;
			node->activate(curr_node,
						   problem,
						   context,
						   run_helper,
						   history->node_histories);
		}

		break;
	}
}

void InfoScope::activate(Problem* problem,
						 RunHelper& run_helper,
						 double& inner_score) {
	// if (this->experiment != NULL) {
	// 	InfoPassThroughExperiment* info_pass_through_experiment = (InfoPassThroughExperiment*)this->experiment;
	// 	info_pass_through_experiment->info_scope_activate(run_helper);
	// }

	vector<ContextLayer> inner_context;
	inner_context.push_back(ContextLayer());

	inner_context.back().scope = this;
	inner_context.back().node = NULL;

	InfoScopeHistory* history = new InfoScopeHistory(this);
	inner_context.back().scope_history = history;

	AbstractNode* curr_node = this->nodes[0];
	while (true) {
		if (curr_node == NULL) {
			break;
		}

		run_helper.num_actions++;
		if (run_helper.num_actions > solution->num_actions_limit) {
			run_helper.exceeded_limit = true;
			break;
		}

		node_activate_helper(curr_node,
							 problem,
							 inner_context,
							 run_helper,
							 history);
	}

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
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != history->node_histories.end()) {
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

	delete history;
}
