#include "info_scope.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void InfoScope::activate(Problem* problem,
						 vector<ContextLayer>& context,
						 RunHelper& run_helper,
						 bool& is_positive) {
	if (this->experiment != NULL) {
		InfoPassThroughExperiment* info_pass_through_experiment = (InfoPassThroughExperiment*)this->experiment;
		info_pass_through_experiment->info_pre_activate(run_helper);
	}

	context.push_back(ContextLayer());

	context.back().scope = this;
	context.back().node = NULL;

	InfoScopeHistory* history = new InfoScopeHistory(this);
	context.back().scope_history = history;

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

		ActionNode* node = (ActionNode*)curr_node;
		node->activate(curr_node,
					   problem,
					   context,
					   run_helper,
					   history->node_histories);
	}

	run_helper.num_decisions++;

	if (this->experiment != NULL) {
		if (run_helper.experiment_histories.size() == 1
				&& run_helper.experiment_histories[0]->experiment == this->experiment) {
			InfoPassThroughExperiment* info_pass_through_experiment = (InfoPassThroughExperiment*)this->experiment;
			bool is_selected = info_pass_through_experiment->info_back_activate(
				problem,
				context,
				run_helper,
				is_positive);
			if (is_selected) {
				return;
			}
		}
	}

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != history->node_histories.end()) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
			input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
		}
	}
	this->network->activate(input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double score = this->network->output->acti_vals[0];
	#endif /* MDEBUG */

	#if defined(MDEBUG) && MDEBUG
	if (run_helper.curr_run_seed%2 == 0) {
		is_positive = true;
	} else {
		is_positive = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	if (score >= 0.0) {
		is_positive = true;
	} else {
		is_positive = false;
	}
	#endif /* MDEBUG */

	delete history;

	context.pop_back();
}
