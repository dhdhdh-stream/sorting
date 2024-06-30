#if defined(MDEBUG) && MDEBUG

#include "new_info_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

bool NewInfoExperiment::capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
		return false;
	}

	run_helper.branch_node_ancestors.insert(this->branch_node);

	run_helper.num_decisions++;

	InfoBranchNodeHistory* branch_node_history = new InfoBranchNodeHistory();
	branch_node_history->index = context.back().scope_history->node_histories.size();
	context.back().scope_history->node_histories[this->branch_node] = branch_node_history;

	AbstractScopeHistory* scope_history;
	this->new_info_scope->explore_activate(problem,
										   context,
										   run_helper,
										   scope_history);

	vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
			this->new_input_node_contexts[i_index]);
		if (it != scope_history->node_histories.end()) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
			new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
		}
	}
	this->new_network->activate(new_input_vals);
	double new_predicted_score = this->new_network->output->acti_vals[0];

	delete scope_history;

	this->verify_scores.push_back(new_predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (decision_is_branch) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		return false;
	}
}

void NewInfoExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		if (this->parent_experiment == NULL) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			vector<AbstractExperiment*> verify_experiments;
			verify_experiments.insert(verify_experiments.begin(), this);
			AbstractExperiment* curr_experiment = this->parent_experiment;
			while (true) {
				if (curr_experiment->parent_experiment == NULL) {
					/**
					 * - don't include root
					 */
					break;
				} else {
					verify_experiments.insert(verify_experiments.begin(), curr_experiment);
					curr_experiment = curr_experiment->parent_experiment;
				}
			}

			this->root_experiment->verify_experiments = verify_experiments;

			this->root_experiment->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

			this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_EXISTING;

			this->state = NEW_INFO_EXPERIMENT_STATE_ROOT_VERIFY;
		}
	}
}

#endif /* MDEBUG */