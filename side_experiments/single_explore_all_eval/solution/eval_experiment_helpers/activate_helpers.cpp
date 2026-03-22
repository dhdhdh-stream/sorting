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
		EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void EvalExperiment::experiment_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 bool& fetch_action,
									 SolutionWrapper* wrapper) {
	EvalExperimentState* experiment_state = (EvalExperimentState*)wrapper->experiment_context.back();

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

		if (wrapper->curr_explore_experiment == NULL) {
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

			if (is_branch) {
				history->hit_branch = true;

				if (!history->is_on) {
					delete experiment_state;
					wrapper->experiment_context.back() = NULL;
					return;
				}
			} else {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		} else {
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
					wrapper->experiment_context.back() = NULL;
					return;
				}
			} else {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
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
		if (history->hit_branch) {
			this->new_sum_scores += target_val;
			this->new_count++;

			this->state_iter++;
		}
	} else {
		if (history->hit_branch) {
			this->existing_sum_scores += target_val;
			this->existing_count++;

			this->state_iter++;
		}
	}

	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
		double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
		double new_score_average = this->new_sum_scores / (double)this->new_count;

		// // temp
		// cout << "this->curr_ramp: " << this->curr_ramp << endl;
		// cout << "existing_score_average: " << existing_score_average << endl;
		// cout << "new_score_average: " << new_score_average << endl;

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

			if (this->curr_ramp == EXPERIMENT_NUM_GEARS) {
				updated_scopes.insert(this->node_context->parent);

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;
			}
		} else {
			this->curr_ramp--;
			if (this->curr_ramp < 0) {
				this->node_context->experiment = NULL;
				delete this;
			}
		}
	}
}
