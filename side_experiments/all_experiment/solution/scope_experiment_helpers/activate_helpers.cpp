#include "scope_experiment.h"

#include <cmath>
#include <iostream>
#include <limits>

#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int CHECK_ITERS_I0 = 1;
const int CHECK_ITERS_I1 = 2;
const int CHECK_ITERS_I2 = 3;
const int CHECK_ITERS_I3 = 4;
#else
const int CHECK_ITERS_I0 = 1;
const int CHECK_ITERS_I1 = 10;
const int CHECK_ITERS_I2 = 100;
const int CHECK_ITERS_I3 = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int STEP_NUM_ITERS = 20;
#else
const int STEP_NUM_ITERS = 2000;
#endif /* MDEBUG */

void ScopeExperiment::check_activate(AbstractNode* experiment_node,
									 vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ScopeExperimentHistory* history = (ScopeExperimentHistory*)wrapper->scope_experiment_history;
	history->is_hit = true;

	switch (this->state) {
	case SCOPE_EXPERIMENT_STATE_INIT:
	case SCOPE_EXPERIMENT_STATE_MEASURE_NEW:
		ScopeExperimentState* new_experiment_state = new ScopeExperimentState(this);
		wrapper->experiment_context.back() = new_experiment_state;
		break;
	}
}

void ScopeExperiment::experiment_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->new_scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);
}

void ScopeExperiment::set_action(int action,
								 SolutionWrapper* wrapper) {
	// do nothing
}

void ScopeExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->node_context.back() = this->exit_next_node;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ScopeExperiment::backprop(double target_val,
							   SolutionWrapper* wrapper,
							   set<Scope*>& updated_scopes) {
	ScopeExperimentHistory* history = (ScopeExperimentHistory*)wrapper->scope_experiment_history;

	switch (this->state) {
	case SCOPE_EXPERIMENT_STATE_INIT:
		if (history->is_hit) {
			this->new_scores.push_back(target_val);

			if ((int)this->new_scores.size() == CHECK_ITERS_I0
					|| (int)this->new_scores.size() == CHECK_ITERS_I1
					|| (int)this->new_scores.size() == CHECK_ITERS_I2) {
				double sum_vals = 0.0;
				for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
					sum_vals += this->new_scores[h_index];
				}
				double average_score = sum_vals / (double)this->new_scores.size();
				#if defined(MDEBUG) && MDEBUG
				if (average_score < this->node_context->val_average || false) {
				#else
				if (average_score < this->node_context->val_average) {
				#endif /* MDEBUG */
					this->node_context->experiment = NULL;
					wrapper->curr_scope_experiment = NULL;
					delete this;
				}
			} else if ((int)this->new_scores.size() == CHECK_ITERS_I3) {
				double sum_vals = 0.0;
				for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
					sum_vals += this->new_scores[h_index];
				}
				double average_score = sum_vals / (double)this->new_scores.size();
				#if defined(MDEBUG) && MDEBUG
				if (average_score >= this->node_context->val_average || rand()%2 == 0) {
				#else
				if (average_score >= this->node_context->val_average) {
				#endif /* MDEBUG */
					this->new_scores.clear();

					this->state = SCOPE_EXPERIMENT_STATE_MEASURE_EXISTING;
					this->state_iter = 0;
				} else {
					this->node_context->experiment = NULL;
					wrapper->curr_scope_experiment = NULL;
					delete this;
				}
			}
		}

		break;
	case SCOPE_EXPERIMENT_STATE_MEASURE_EXISTING:
		if (history->is_hit) {
			this->existing_scores.push_back(target_val);

			this->state_iter++;
			if (this->state_iter >= STEP_NUM_ITERS) {
				this->state = SCOPE_EXPERIMENT_STATE_MEASURE_NEW;
				this->state_iter = 0;
			}
		}
		break;
	case SCOPE_EXPERIMENT_STATE_MEASURE_NEW:
		this->total_sum_scores += target_val;
		this->total_count++;

		if (history->is_hit) {
			this->new_scores.push_back(target_val);

			this->state_iter++;
			if (this->state_iter >= STEP_NUM_ITERS) {
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

				double average_hits_per_run = (double)this->new_scores.size() / (double)this->total_count;

				this->local_improvement = new_score_average - existing_score_average;
				this->global_improvement = average_hits_per_run * this->local_improvement;
				double t_score = this->local_improvement
					/ (this->score_standard_deviation / sqrt((double)this->new_scores.size()));

				// temp
				cout << "new_score_average: " << new_score_average << endl;
				cout << "this->new_scores.size(): " << this->new_scores.size() << endl;
				cout << "existing_score_average: " << existing_score_average << endl;
				cout << "this->local_improvement: " << this->local_improvement << endl;
				cout << "this->global_improvement: " << this->global_improvement << endl;
				cout << "t_score: " << t_score << endl;

				#if defined(MDEBUG) && MDEBUG
				if (t_score >= SUCCESS_T_SCORE || rand()%3 == 0) {
				#else
				if (t_score >= SUCCESS_T_SCORE) {
				#endif /* MDEBUG */
					bool is_success = false;
					if (wrapper->solution->scope_last_scores.size() >= SCOPE_MIN_NUM_LAST_TRACK) {
						int num_better_than = 0;
						for (list<double>::iterator it = wrapper->solution->scope_last_scores.begin();
								it != wrapper->solution->scope_last_scores.end(); it++) {
							if (this->global_improvement >= *it) {
								num_better_than++;
							}
						}

						double target_better_than = SCOPE_LAST_BETTER_THAN_RATIO * (double)wrapper->solution->scope_last_scores.size();

						if (num_better_than >= target_better_than) {
							is_success = true;
						}

						if (wrapper->solution->scope_last_scores.size() >= SCOPE_NUM_LAST_TRACK) {
							wrapper->solution->scope_last_scores.pop_front();
						}
						wrapper->solution->scope_last_scores.push_back(this->global_improvement);
					} else {
						wrapper->solution->scope_last_scores.push_back(this->global_improvement);
					}

					#if defined(MDEBUG) && MDEBUG
					if (is_success || rand()%3 != 0) {
					#else
					if (is_success) {
					#endif /* MDEBUG */
						updated_scopes.insert(this->node_context->parent);

						this->node_context->experiment = NULL;
						wrapper->curr_scope_experiment = NULL;

						add(wrapper);

						delete this;
					} else {
						this->node_context->experiment = NULL;
						wrapper->curr_scope_experiment = NULL;
						delete this;
					}
				#if defined(MDEBUG) && MDEBUG
				} else if (t_score < FAIL_T_SCORE && rand()%2 == 0) {
				#else
				} else if (t_score < FAIL_T_SCORE) {
				#endif /* MDEBUG */
					this->node_context->experiment = NULL;
					wrapper->curr_scope_experiment = NULL;
					delete this;
				} else {
					this->state = SCOPE_EXPERIMENT_STATE_MEASURE_EXISTING;
					this->state_iter = 0;
				}
			}
		}

		break;
	}
}
