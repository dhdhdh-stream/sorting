#if defined(MDEBUG) && MDEBUG

#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::capture_verify_activate(AbstractNode*& curr_node,
											   Problem* problem,
											   vector<ContextLayer>& context,
											   RunHelper& run_helper,
											   ScopeHistory* scope_history) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;

	run_helper.num_actions++;

	if (this->is_local) {
		run_helper.num_analyze += problem_type->num_obs();

		vector<double> input_vals = problem->get_observations();
		this->network->activate(input_vals);
	} else {
		run_helper.num_analyze += (int)this->inputs.size();

		vector<double> input_vals(this->inputs.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->inputs.size(); i_index++) {
			fetch_input_helper(scope_history,
							   this->inputs[i_index],
							   0,
							   input_vals[i_index]);
		}
		this->network->activate(input_vals);
	}

	double predicted_score = this->network->output->acti_vals[0];

	this->verify_scores.push_back(predicted_score);

	cout << "run_helper.starting_run_seed: " << run_helper.starting_run_seed << endl;
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	cout << "context scope" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].scope_id << endl;
	}
	cout << "context node" << endl;
	for (int c_index = 0; c_index < (int)context.size()-1; c_index++) {
		cout << c_index << ": " << context[c_index].node_id << endl;
	}

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	cout << "decision_is_branch: " << decision_is_branch << endl;

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
	}

	return false;
}

void BranchExperiment::capture_verify_backprop() {
	if (this->verify_problems[this->state_iter] != NULL) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */