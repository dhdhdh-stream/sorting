#include "eval_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int EVAL_EPOCH_NUM_ITERS = 20;
#else
const int EVAL_EPOCH_NUM_ITERS = 4000;
#endif /* MDEBUG */

void EvalExperiment::eval_backprop(double target_val,
								   EvalExperimentHistory* history,
								   SolutionWrapper* wrapper) {
	if (history->is_on) {
		this->new_scores.push_back(target_val);
	} else {
		this->existing_scores.push_back(target_val);
	}

	this->state_iter++;
	#if defined(MDEBUG) && MDEBUG
	if (this->state_iter >= EVAL_EPOCH_NUM_ITERS
			&& this->existing_scores.size() >= 2
			&& this->new_scores.size() >= 2) {
	#else
	if (this->state_iter >= EVAL_EPOCH_NUM_ITERS) {
	#endif /* MDEBUG */
		double existing_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_score += this->existing_scores[h_index];
		}
		double existing_score_average = existing_sum_score / (double)this->existing_scores.size();

		double existing_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
			existing_sum_variance += (this->existing_scores[h_index] - existing_score_average)
				* (this->existing_scores[h_index] - existing_score_average);
		}
		double existing_score_standard_deviation = sqrt(existing_sum_variance / (double)this->existing_scores.size());

		double new_sum_score = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_score += this->new_scores[h_index];
		}
		double new_score_average = new_sum_score / (double)this->new_scores.size();

		double new_sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			new_sum_variance += (this->new_scores[h_index] - new_score_average)
				* (this->new_scores[h_index] - new_score_average);
		}
		double new_score_standard_deviation = sqrt(new_sum_variance / (double)this->new_scores.size());

		double score_improvement = new_score_average - existing_score_average;
		double existing_score_standard_error = existing_score_standard_deviation / sqrt((double)this->existing_scores.size());
		double new_score_standard_error = new_score_standard_deviation / sqrt((double)this->new_scores.size());
		double score_t_score = score_improvement / sqrt(
			existing_score_standard_error * existing_score_standard_error
				+ new_score_standard_error * new_score_standard_error);

		this->existing_scores.clear();
		this->new_scores.clear();

		this->state_iter = 0;

		#if defined(MDEBUG) && MDEBUG
		if (score_t_score >= -0.674 || rand()%3 != 0) {
		#else
		if (score_t_score >= -0.674) {
		#endif /* MDEBUG */
			this->curr_ramp++;
			if (this->curr_ramp >= EVAL_EXPERIMENT_NUM_GEARS-1) {
				cout << "success" << endl;
				cout << "existing_score_average: " << existing_score_average << endl;
				cout << "new_score_average: " << new_score_average << endl;
				cout << "score_t_score: " << score_t_score << endl;
				cout << endl;

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;

				measure_score(wrapper);
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
