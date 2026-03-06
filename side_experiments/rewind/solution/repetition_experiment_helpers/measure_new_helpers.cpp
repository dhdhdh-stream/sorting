#include "repetition_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void RepetitionExperiment::measure_new_check_activate(
		SolutionWrapper* wrapper) {
	RepetitionExperimentState* new_experiment_state = new RepetitionExperimentState(this);
	wrapper->experiment_context.back() = new_experiment_state;
}

void RepetitionExperiment::measure_new_step(vector<double>& obs,
											int& action,
											bool& is_next,
											SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->new_scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);
}

void RepetitionExperiment::measure_new_exit_step(SolutionWrapper* wrapper) {
	RepetitionExperimentState* experiment_state = (RepetitionExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->node_context.back() = this->exit_next_node;

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void RepetitionExperiment::measure_new_backprop(
		double target_val,
		SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	RepetitionExperimentHistory* history = (RepetitionExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->new_scores.push_back(target_val);

		this->state_iter++;
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			double existing_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_vals += this->existing_scores[h_index];
			}
			double existing_true = existing_sum_vals / (double)this->existing_scores.size();
			double new_sum_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_vals += this->new_scores[h_index];
			}
			double new_true = new_sum_vals / (double)this->new_scores.size();
			this->local_improvement = new_true - existing_true;
			double average_hits_per_run = (double)this->new_scores.size() / (double)this->total_count;
			this->global_improvement = average_hits_per_run * this->local_improvement;
			double sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				sum_variance += (this->new_scores[h_index] - new_true) * (this->new_scores[h_index] - new_true);
			}
			this->score_standard_deviation = sqrt(sum_variance / (double)this->new_scores.size());

			double t_score = this->local_improvement
				/ (this->score_standard_deviation / sqrt((double)this->new_scores.size()));

			// // temp
			// cout << "existing_true: " << existing_true << endl;
			// cout << "new_true: " << new_true << endl;
			// cout << "this->new_scores.size(): " << this->new_scores.size() << endl;
			// cout << "average_hits_per_run: " << average_hits_per_run << endl;
			// cout << "t_score: " << t_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (t_score >= SUCCESS_T_SCORE || rand()%3 == 0) {
			#else
			if (t_score >= SUCCESS_T_SCORE) {
			#endif /* MDEBUG */
				bool is_success = false;
				if (wrapper->solution->repetition_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->repetition_last_scores.begin();
							it != wrapper->solution->repetition_last_scores.end(); it++) {
						if (this->global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->repetition_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->repetition_last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->repetition_last_scores.pop_front();
					}
					wrapper->solution->repetition_last_scores.push_back(this->global_improvement);
				} else {
					wrapper->solution->repetition_last_scores.push_back(this->global_improvement);
				}

				if (is_success) {
					cout << "RepetitionExperiment" << endl;
					cout << "this->node_context->parent->id: " << this->node_context->parent->id << endl;
					cout << "this->node_context->id: " << this->node_context->id << endl;

					cout << "this->local_improvement: " << this->local_improvement << endl;
					cout << "average_hits_per_run: " << average_hits_per_run << endl;
					cout << "this->global_improvement: " << this->global_improvement << endl;

					cout << endl;

					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			#if defined(MDEBUG) && MDEBUG
			} else if (t_score < FAIL_T_SCORE && rand()%2 == 0) {
			#else
			} else if (t_score < FAIL_T_SCORE) {
			#endif /* MDEBUG */
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->state = REPETITION_EXPERIMENT_STATE_MEASURE_EXISTING;
				this->state_iter = 0;
			}
		}
	}
}
