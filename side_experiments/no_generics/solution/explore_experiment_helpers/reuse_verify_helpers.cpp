#if defined(MDEBUG) && MDEBUG

#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "noop_node.h"
#include "pass_through_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

const int VERIFY_NUM_ITERS = 10;

void ExploreExperiment::reuse_verify_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	history->verify_state_vals.push_back(wrapper->states.back());

	bool is_branch;
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

	if (is_branch) {
		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::reuse_verify_step(vector<double>& obs,
										  int& action,
										  bool& is_next,
										  SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			Scope* scope = this->scope_context->child_scopes[this->best_indexes[experiment_state->step_index]];
			ScopeHistory* inner_scope_history = new ScopeHistory(scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(scope->num_states);
			wrapper->states.back().setConstant(0.0);
		}
	}
}

void ExploreExperiment::reuse_verify_callback(vector<double>& obs,
											  SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	experiment_state->step_index++;
}

void ExploreExperiment::reuse_verify_exit_step(vector<double>& obs,
											   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::reuse_verify_backprop(double target_val,
											  ExploreExperimentHistory* history,
											  SolutionWrapper* wrapper) {
	this->verify_problems.push_back(wrapper->problem->copy_and_reset());
	this->verify_run_seeds.push_back(wrapper->starting_run_seed);
	this->verify_state_vals.insert(this->verify_state_vals.end(),
		history->verify_state_vals.begin(), history->verify_state_vals.end());

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_MEASURE_NUM_DATAPOINTS) {
		add(false,
			wrapper);
	}
}

#endif /* MDEBUG */