#if defined(MDEBUG) && MDEBUG

#include "chase_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void ChaseExperiment::capture_verify_check_activate(
		SolutionWrapper* wrapper) {
	if (this->verify_problems[this->state_iter] == NULL) {
		this->verify_problems[this->state_iter] = wrapper->problem->copy_and_reset();
	}
	this->verify_seeds[this->state_iter] = wrapper->starting_run_seed;

	ChaseExperimentState* new_experiment_state = new ChaseExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ChaseExperiment::capture_verify_step(vector<double>& obs,
										  int& action,
										  bool& is_next,
										  SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		this->new_signal_network->activate(obs);

		this->verify_scores.push_back(this->new_signal_network->output->acti_vals[0]);

		cout << "wrapper->starting_run_seed: " << wrapper->starting_run_seed << endl;
		cout << "wrapper->curr_run_seed: " << wrapper->curr_run_seed << endl;
		wrapper->problem->print();

		bool decision_is_branch;
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

		cout << "decision_is_branch: " << decision_is_branch << endl;

		if (!decision_is_branch) {
			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeNode* scope_node = (ScopeNode*)this->best_new_nodes[experiment_state->step_index];

			ScopeHistory* scope_history = wrapper->scope_histories.back();

			ScopeNodeHistory* history = new ScopeNodeHistory(scope_node);
			scope_history->node_histories[scope_node->id] = history;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			history->scope_history = inner_scope_history;
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ChaseExperiment::capture_verify_exit_step(SolutionWrapper* wrapper) {
	ChaseExperimentState* experiment_state = (ChaseExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	vector<double> obs = wrapper->problem->get_observations();
	wrapper->scope_histories.back()->obs_history = obs;

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ChaseExperiment::capture_verify_backprop(SolutionWrapper* wrapper) {
	ChaseExperimentHistory* history = (ChaseExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		this->state_iter++;
		if (this->state_iter >= NUM_VERIFY_SAMPLES) {
			this->result = EXPERIMENT_RESULT_SUCCESS;
		}
	}
}

#endif /* MDEBUG */