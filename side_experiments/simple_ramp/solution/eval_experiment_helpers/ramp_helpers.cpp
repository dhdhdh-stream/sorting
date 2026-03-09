#include "eval_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

void EvalExperiment::ramp_check_activate(
		AbstractNode* experiment_node,
		vector<double>& obs,
		SolutionWrapper* wrapper,
		EvalExperimentHistory* history) {
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

		if (history->is_on) {
			EvalExperimentState* new_experiment_state = new EvalExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void EvalExperiment::ramp_backprop(double target_val,
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
		double existing_score_average = this->existing_sum_scores / this->existing_count;
		double new_score_average = this->new_sum_scores / this->new_count;

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

			if (this->curr_ramp == RAMP_GEAR) {
				this->state = EVAL_EXPERIMENT_STATE_RAMP;
			} else if (this->curr_ramp == EXPERIMENT_NUM_GEARS) {
				updated_scopes.insert(this->node_context->parent);

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;

				wrapper->curr_num_eval--;
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

				wrapper->curr_num_eval--;
			}
		}
	}
}
