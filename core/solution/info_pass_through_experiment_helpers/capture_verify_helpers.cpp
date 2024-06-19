#if defined(MDEBUG) && MDEBUG

#include "info_pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "utilities.h"

using namespace std;

void InfoPassThroughExperiment::capture_verify_activate(
		AbstractNode*& curr_node) {
	if (this->actions.size() == 0) {
		curr_node = this->exit_next_node;
	} else {
		curr_node = this->actions[0];
	}
}

void InfoPassThroughExperiment::capture_verify_info_back_activate(
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		bool& is_positive) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = context.back().scope_history->node_histories.find(
			this->new_input_node_contexts[i_index]);
		if (it != context.back().scope_history->node_histories.end()) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
			new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
		}
	}
	this->new_network->activate(new_input_vals);
	double new_predicted_score = this->new_network->output->acti_vals[0];

	this->verify_scores.push_back(new_predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	if (run_helper.curr_run_seed%2 == 0) {
		is_positive = true;
	} else {
		is_positive = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
}

void InfoPassThroughExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}

#endif /* MDEBUG */