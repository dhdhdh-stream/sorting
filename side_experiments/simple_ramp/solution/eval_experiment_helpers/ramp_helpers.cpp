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
	if (history->hit_branch) {
		this->num_branch++;
	} else {
		this->num_original++;
		if (this->num_original == BRANCH_RATIO_CHECK_ITER) {
			double branch_ratio = (double)this->num_branch / ((double)this->num_original + (double)this->num_branch);
			if (branch_ratio < BRANCH_MIN_RATIO) {
				this->node_context->experiment = NULL;
				delete this;
				return;
			}
		}
	}

	if (history->is_on) {
		if (history->hit_branch) {
			this->new_scores.push_back(target_val);

			this->state_iter++;
		}
	} else {
		if (history->hit_branch) {
			this->existing_scores.push_back(target_val);

			this->state_iter++;
		}
	}

	switch (this->state) {
	case EVAL_EXPERIMENT_STATE_INIT:
	case EVAL_EXPERIMENT_STATE_RAMP:
		if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
			double existing_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_vals += this->existing_scores[h_index];
			}
			double existing_score_average = existing_sum_vals / (double)this->existing_scores.size();
			double new_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_vals += this->new_scores[h_index];
			}
			double new_score_average = new_sum_vals / (double)this->new_scores.size();

			// temp
			cout << "this->curr_ramp: " << this->curr_ramp << endl;
			cout << "existing_score_average: " << existing_score_average << endl;
			cout << "new_score_average: " << new_score_average << endl;

			this->existing_scores.clear();
			this->new_scores.clear();

			this->state_iter = 0;

			#if defined(MDEBUG) && MDEBUG
			if ((this->measure_status != MEASURE_STATUS_FAIL && new_score_average >= existing_score_average) || rand()%3 != 0) {
			#else
			if (this->measure_status != MEASURE_STATUS_FAIL && new_score_average >= existing_score_average) {
			#endif /* MDEBUG */
				this->curr_ramp++;
				this->num_fail = 0;

				if (this->curr_ramp == RAMP_GEAR) {
					this->state = EVAL_EXPERIMENT_STATE_RAMP;
				} else if (this->curr_ramp == MEASURE_GEAR
						&& this->measure_status == MEASURE_STATUS_N_A) {
					this->starting_experiment_iter = wrapper->experiment_iter;

					this->state = EVAL_EXPERIMENT_STATE_MEASURE;
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
		break;
	case EVAL_EXPERIMENT_STATE_MEASURE:
		if (this->state_iter >= MEASURE_EPOCH_NUM_ITERS) {
			double existing_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_vals += this->existing_scores[h_index];
			}
			double existing_score_average = existing_sum_vals / (double)this->existing_scores.size();
			double new_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_vals += this->new_scores[h_index];
			}
			double new_score_average = new_sum_vals / (double)this->new_scores.size();

			double sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				sum_variance += (this->new_scores[h_index] - new_score_average) * (this->new_scores[h_index] - new_score_average);
			}
			this->score_standard_deviation = sqrt(sum_variance / (double)this->new_scores.size());

			this->existing_scores.clear();
			this->new_scores.clear();

			int total_iters = wrapper->experiment_iter - this->starting_experiment_iter;
			if (total_iters < 0) {
				total_iters += numeric_limits<int>::max();
			}
			double average_hits_per_run = (double)MEASURE_EPOCH_NUM_ITERS / (double)total_iters;

			this->local_improvement = new_score_average - existing_score_average;
			this->global_improvement = average_hits_per_run * this->local_improvement;
			double t_score = this->local_improvement
				/ (this->score_standard_deviation / sqrt(MEASURE_EPOCH_NUM_ITERS / 2.0));

			// temp
			cout << "new_score_average: " << new_score_average << endl;
			cout << "existing_score_average: " << existing_score_average << endl;
			cout << "this->local_improvement: " << this->local_improvement << endl;
			cout << "this->global_improvement: " << this->global_improvement << endl;
			cout << "t_score: " << t_score << endl;

			this->state_iter = 0;

			bool is_success = false;
			#if defined(MDEBUG) && MDEBUG
			if (t_score >= 1.645 || rand()%2 == 0) {
			#else
			if (t_score >= 1.645) {
			#endif /* MDEBUG */
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->last_scores.begin();
						it != wrapper->solution->last_scores.end(); it++) {
					if (this->global_improvement >= *it) {
						num_better_than++;
					}
				}

				int target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->last_scores.size() >= NUM_LAST_TRACK) {
					wrapper->solution->last_scores.pop_front();
				}
				wrapper->solution->last_scores.push_back(this->global_improvement);
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_success || rand()%3 != 0) {
			#else
			if (is_success) {
			#endif /* MDEBUG */
				this->measure_status = MEASURE_STATUS_SUCCESS;

				this->state = EVAL_EXPERIMENT_STATE_RAMP;

				this->curr_ramp++;
			} else {
				this->measure_status = MEASURE_STATUS_FAIL;

				this->state = EVAL_EXPERIMENT_STATE_RAMP;

				this->curr_ramp--;
			}
		}
		break;
	}
}
