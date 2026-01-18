#include "eval_experiment.h"

#include <iostream>

#include "constants.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

const int RAMP_ITER = 3;

void EvalExperiment::check_activate(AbstractNode* experiment_node,
									vector<double>& obs,
									SolutionWrapper* wrapper) {
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
		this->new_network->activate(obs);

		bool decision_is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (this->new_network->output->acti_vals[0] >= 0.0) {
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
	if (history->is_on) {
		this->new_sum_scores += target_val;
		this->new_count++;
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
		double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
		double new_score_average = this->new_sum_scores / (double)this->new_count;

		// temp
		cout << "this->curr_ramp: " << this->curr_ramp << endl;
		cout << "existing_score_average: " << existing_score_average << endl;
		cout << "new_score_average: " << new_score_average << endl;

		this->existing_sum_scores = 0.0;
		this->existing_count = 0;
		this->new_sum_scores = 0.0;
		this->new_count = 0;

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (new_score_average >= existing_score_average || rand()%3 != 0) {
		#else
		if (new_score_average >= existing_score_average) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			this->num_fail = 0;

			if (this->curr_ramp == RAMP_ITER) {
				this->state = EVAL_EXPERIMENT_STATE_RAMP;
			} else if (this->curr_ramp == EXPERIMENT_NUM_GEARS) {
				updated_scopes.insert(this->node_context->parent);

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;
			}
		} else {
			switch (this->state) {
			case EVAL_EXPERIMENT_STATE_INIT:
				this->num_fail++;
				if (this->num_fail >= 2) {
					this->curr_ramp--;
					this->num_fail = 0;
				}
				break;
			case EVAL_EXPERIMENT_STATE_RAMP:
				this->curr_ramp--;
				break;
			}

			if (this->curr_ramp < 0) {
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
