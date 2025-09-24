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

void EvalExperiment::ramp_backprop(double target_val,
								   EvalExperimentHistory* history,
								   SolutionWrapper* wrapper,
								   set<Scope*>& updated_scopes) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);
	} else {
		this->existing_scores.push_back(target_val);
	}

	this->state_iter++;
	#if defined(MDEBUG) && MDEBUG
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS
			&& this->existing_scores.size() >= 2
			&& this->new_scores.size() >= 2) {
	#else
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
	#endif /* MDEBUG */
		switch (this->result) {
		case EVAL_RESULT_NA:
			{
				double existing_sum_score = 0.0;
				for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
					existing_sum_score += this->existing_scores[h_index];
				}
				double existing_score_average = existing_sum_score / (double)this->existing_scores.size();

				double new_sum_score = 0.0;
				for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
					new_sum_score += this->new_scores[h_index];
				}
				double new_score_average = new_sum_score / (double)this->new_scores.size();

				double improvement = new_score_average - existing_score_average;

				this->existing_scores.clear();
				this->new_scores.clear();

				this->state_iter = 0;

				#if defined(MDEBUG) && MDEBUG
				if (improvement >= 0.0 || rand()%3 != 0) {
				#else
				if (improvement >= 0.0) {
				#endif /* MDEBUG */
					this->curr_ramp++;
					this->num_fail = 0;

					if (this->curr_ramp == EVAL_GEAR) {
						int num_better_than = 0;
						for (list<double>::iterator it = wrapper->solution->last_experiment_scores.begin();
								it != wrapper->solution->last_experiment_scores.end(); it++) {
							if (improvement >= *it) {
								num_better_than++;
							}
						}

						int target_better_than = LAST_EXPERIMENT_BETTER_THAN_RATIO * (double)wrapper->solution->last_experiment_scores.size();

						if (wrapper->solution->last_experiment_scores.size() >= NUM_LAST_EXPERIMENT_TRACK) {
							wrapper->solution->last_experiment_scores.pop_front();
						}
						wrapper->solution->last_experiment_scores.push_back(improvement);

						if (num_better_than >= target_better_than) {
							cout << "success" << endl;
							cout << "existing_score_average: " << existing_score_average << endl;
							cout << "new_score_average: " << new_score_average << endl;
							cout << "this->scope_context->id: " << this->scope_context->id << endl;
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
						} else {
							this->result = EVAL_RESULT_FAIL;
						}
					}
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
			break;
		case EVAL_RESULT_FAIL:
			this->curr_ramp--;
			if (this->curr_ramp < 0) {
				this->node_context->experiment = NULL;
				delete this;
			}
			break;
		case EVAL_RESULT_SUCCESS:
			this->curr_ramp++;
			if (this->curr_ramp >= EXPERIMENT_NUM_GEARS-1) {
				updated_scopes.insert(this->scope_context);

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;

				measure_score(wrapper);
			}
		}
	}
}
