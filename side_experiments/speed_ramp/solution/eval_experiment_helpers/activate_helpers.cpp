#include "eval_experiment.h"

#include "constants.h"
#include "helpers.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void EvalExperiment::check_activate(AbstractNode* experiment_node,
									bool is_branch,
									SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		EvalExperimentHistory* history;
		map<EvalExperiment*, EvalExperimentHistory*>::iterator it =
			wrapper->eval_histories.find(this);
		if (it == wrapper->eval_histories.end()) {
			history = new EvalExperimentHistory(this);
			wrapper->eval_histories[this] = history;
		} else {
			history = it->second;
		}

		if (history->is_on) {
			ScopeHistory* scope_history = wrapper->scope_histories.back();

			if (this->select_percentage == 1.0) {
				EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
				new_experiment_state->step_index = 0;
				wrapper->experiment_context.back() = new_experiment_state;
			} else {
				double sum_vals = this->new_average_score;

				for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
					double val;
					bool is_on;
					fetch_input_helper(scope_history,
									   this->new_inputs[i_index],
									   0,
									   val,
									   is_on);
					if (is_on) {
						double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
						sum_vals += this->new_weights[i_index] * normalized_val;
					}
				}

				if (this->new_network != NULL) {
					vector<double> input_vals(this->new_network_inputs.size());
					vector<bool> input_is_on(this->new_network_inputs.size());
					for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
						double val;
						bool is_on;
						fetch_input_helper(scope_history,
										   this->new_network_inputs[i_index],
										   0,
										   val,
										   is_on);
						input_vals[i_index] = val;
						input_is_on[i_index] = is_on;
					}
					this->new_network->activate(input_vals,
												input_is_on);
					sum_vals += this->new_network->output->acti_vals[0];
				}

				bool decision_is_branch;
				#if defined(MDEBUG) && MDEBUG
				if (wrapper->curr_run_seed%2 == 0) {
					decision_is_branch = true;
				} else {
					decision_is_branch = false;
				}
				wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
				#else
				if (sum_vals >= 0.0) {
					decision_is_branch = true;
				} else {
					decision_is_branch = false;
				}
				#endif /* MDEBUG */

				if (decision_is_branch) {
					EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
					new_experiment_state->step_index = 0;
					wrapper->experiment_context.back() = new_experiment_state;
				}
			}
		}
	}
}

void EvalExperiment::experiment_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void EvalExperiment::set_action(int action,
								SolutionWrapper* wrapper) {
	// do nothing
}

void EvalExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void EvalExperiment::backprop(double target_val,
							  EvalExperimentHistory* history,
							  SolutionWrapper* wrapper,
							  set<Scope*>& updated_scopes) {
	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_INITIAL:
		initial_backprop(target_val,
						 history,
						 wrapper);
		break;
	case EVAL_EXPERIMENT_STATE_EVAL:
		eval_backprop(target_val,
					  history,
					  wrapper,
					  updated_scopes);
		break;
	}
}
