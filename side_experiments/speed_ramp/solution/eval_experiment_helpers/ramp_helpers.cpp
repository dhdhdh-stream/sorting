#include "eval_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_ITERS = 200;
#else
const int MEASURE_NUM_ITERS = 8000;
#endif /* MDEBUG */

void EvalExperiment::ramp_backprop(double target_val,
								   EvalExperimentHistory* history,
								   SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_sum_scores += target_val;
		this->new_count++;
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	bool is_done;
	if (this->curr_ramp == EVAL_GEAR-1) {
		if (this->state_iter >= MEASURE_NUM_ITERS) {
			is_done = true;
		} else {
			is_done = false;
		}
	} else {
		if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
			is_done = true;
		} else {
			is_done = false;
		}
	}
	if (is_done) {
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

		if (this->curr_ramp == EVAL_GEAR-1) {
			#if defined(MDEBUG) && MDEBUG
			if (new_score_average >= existing_score_average || rand()%3 != 0) {
			#else
			if (new_score_average >= existing_score_average) {
			#endif /* MDEBUG */
				double improvement = new_score_average - existing_score_average;

				// temp
				cout << "improvement: " << improvement << endl;

				bool is_success;
				if (wrapper->solution->last_experiment_scores.size() >= MIN_NUM_LAST_EXPERIMENT_TRACK) {
					int num_better_than = 0;
					// temp
					cout << "last_experiment_scores:";
					for (list<double>::iterator it = wrapper->solution->last_experiment_scores.begin();
							it != wrapper->solution->last_experiment_scores.end(); it++) {
						// temp
						cout << " " << *it;
						if (improvement >= *it) {
							num_better_than++;
						}
					}
					// temp
					cout << endl;

					int target_better_than = LAST_EXPERIMENT_BETTER_THAN_RATIO * (double)wrapper->solution->last_experiment_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					} else {
						is_success = false;
					}

					if (wrapper->solution->last_experiment_scores.size() >= NUM_LAST_EXPERIMENT_TRACK) {
						wrapper->solution->last_experiment_scores.pop_front();
					}
					wrapper->solution->last_experiment_scores.push_back(improvement);
				} else {
					is_success = false;

					wrapper->solution->last_experiment_scores.push_back(improvement);
				}

				if (is_success) {
					cout << "success" << endl;
					cout << "existing_score_average: " << existing_score_average << endl;
					cout << "new_score_average: " << new_score_average << endl;
					cout << "this->node_context->parent->id: " << this->node_context->parent->id << endl;
					cout << "this->node_context->id: " << this->node_context->id << endl;
					cout << "this->is_branch: " << this->is_branch << endl;
					cout << "new explore path:";
					for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
						if (this->step_types[s_index] == STEP_TYPE_ACTION) {
							cout << " " << this->actions[s_index];
						} else {
							cout << " E" << this->scopes[s_index]->id;
						}
					}
					cout << endl;

					if (this->exit_next_node == NULL) {
						cout << "this->exit_next_node->id: " << -1 << endl;
					} else {
						cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
					}

					cout << "this->select_percentage: " << this->select_percentage << endl;

					cout << endl;

					this->result = EVAL_RESULT_SUCCESS;

					this->curr_ramp++;

					this->state = EVAL_EXPERIMENT_STATE_WRAPUP;
					this->state_iter = 0;
				} else {
					this->result = EVAL_RESULT_FAIL;

					this->curr_ramp--;

					this->state = EVAL_EXPERIMENT_STATE_WRAPUP;
					this->state_iter = 0;
				}
			} else {
				this->result = EVAL_RESULT_FAIL;

				this->curr_ramp--;

				this->state = EVAL_EXPERIMENT_STATE_WRAPUP;
				this->state_iter = 0;
			}
		} else {
			#if defined(MDEBUG) && MDEBUG
			if (new_score_average >= existing_score_average || rand()%3 != 0) {
			#else
			if (new_score_average >= existing_score_average) {
			#endif /* MDEBUG */
				this->curr_ramp++;
				this->num_fail = 0;
			} else {
				this->num_fail++;
				if (this->num_fail >= 2) {
					this->curr_ramp--;
					this->num_fail = 0;

					if (this->curr_ramp < 0) {
						this->node_context->experiment = NULL;
						delete this;
					}
				}
			}
		}
	}
}
