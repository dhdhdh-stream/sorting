#include "eval_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void EvalExperiment::experiment_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		switch (this->state) {
		case EVAL_EXPERIMENT_STATE_RAMP:
		case EVAL_EXPERIMENT_STATE_MEASURE:
			ramp_check_activate(wrapper);
			break;
		}
	}
}

void EvalExperiment::experiment_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_step(obs,
				  action,
				  is_next,
				  fetch_action,
				  wrapper);
		break;
	}
}

void EvalExperiment::set_action(int action,
								SolutionWrapper* wrapper) {
	// do nothing
}

void EvalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_exit_step(wrapper);
		break;
	}
}

void EvalExperiment::backprop(double target_val,
							  EvalExperimentHistory* history,
							  SolutionWrapper* wrapper,
							  set<Scope*>& updated_scopes) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_RAMP:
	case EVAL_EXPERIMENT_STATE_MEASURE:
		ramp_backprop(target_val,
					  history,
					  wrapper,
					  updated_scopes);
		break;
	}
}

void EvalExperiment::result_check_activate(AbstractNode* experiment_node,
										   vector<double>& obs,
										   SolutionWrapper* wrapper) {
	EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->result_experiment_context.back() = new_experiment_state;
}

void EvalExperiment::result_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 bool& fetch_action,
								 SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->result_experiment_context.back();

	if (experiment_state->step_index == 0) {
		EvalExperimentHistory* history;
		map<EvalExperiment*, EvalExperimentHistory*>::iterator it =
			wrapper->eval_experiment_histories.find(this);
		if (it == wrapper->eval_experiment_histories.end()) {
			history = new EvalExperimentHistory(this);
			wrapper->eval_experiment_histories[this] = history;
		} else {
			history = it->second;
		}

		if (history->is_on) {
			bool is_branch = true;
			for (int n_index = 0; n_index < (int)this->new_networks.size(); n_index++) {
				this->new_networks[n_index]->activate(obs);
				if (this->new_networks[n_index]->output->acti_vals[0] < 0.0) {
					is_branch = false;
					break;
				}
			}

			#if defined(MDEBUG) && MDEBUG
			if (wrapper->curr_run_seed%2 == 0) {
				is_branch = true;
			} else {
				is_branch = false;
			}
			wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
			#endif /* MDEBUG */

			if (!is_branch) {
				delete experiment_state;
				wrapper->result_experiment_context.back() = NULL;
				return;
			}
		} else {
			delete experiment_state;
			wrapper->result_experiment_context.back() = NULL;
			return;
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->result_node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->result_experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->result_num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->result_scope_histories.push_back(inner_scope_history);
			wrapper->result_node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->result_experiment_context.push_back(NULL);
		}
	}
}

void EvalExperiment::result_set_action(int action,
									   SolutionWrapper* wrapper) {
	// do nothing
}

void EvalExperiment::result_exit_step(SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->result_experiment_context[wrapper->result_experiment_context.size() - 2];

	delete wrapper->result_scope_histories.back();

	wrapper->result_scope_histories.pop_back();
	wrapper->result_node_context.pop_back();
	wrapper->result_experiment_context.pop_back();

	experiment_state->step_index++;
}
