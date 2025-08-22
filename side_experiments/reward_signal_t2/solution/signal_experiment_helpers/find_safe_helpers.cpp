#include "signal_experiment.h"

#include <cmath>
#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "explore.h"
#include "globals.h"
#include "helpers.h"
#include "problem.h"
#include "scope.h"

using namespace std;

const int CHECK_1_NUM_ITERS = 1;
const int CHECK_2_NUM_ITERS = 10;
const int CHECK_3_NUM_ITERS = 100;
const int CHECK_4_NUM_ITERS = 1000;

const int MAX_FIND_SAFE_ITERS = 200;

void SignalExperiment::find_safe_backprop(
		double target_val) {
	this->new_scores.push_back(target_val);

	bool is_fail = false;
	if (this->new_scores.size() == CHECK_1_NUM_ITERS
			|| this->new_scores.size() == CHECK_2_NUM_ITERS
			|| this->new_scores.size() == CHECK_3_NUM_ITERS
			|| this->new_scores.size() == CHECK_4_NUM_ITERS) {
		double sum_new_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
			sum_new_vals += this->new_scores[h_index];
		}
		double new_average = sum_new_vals / (double)this->new_scores.size();

		double new_improvement = new_average - this->existing_average;
		double t_score = new_improvement / (this->existing_standard_deviation / sqrt((double)this->new_scores.size()));

		#if defined(MDEBUG) && MDEBUG
		if (t_score < -0.674 && rand()%4 == 0) {
			is_fail = true;
		}
		#else
		if (t_score < -0.674) {
			is_fail = true;
		}
		#endif /* MDEBUG */
	}

	if (is_fail) {
		this->state_iter++;
		if (this->state_iter >= MAX_FIND_SAFE_ITERS) {
			this->state = SIGNAL_EXPERIMENT_STATE_DONE;
		} else {
			this->pre_actions.clear();
			this->post_actions.clear();

			this->new_scores.clear();

			for (int s_index = 0; s_index < (int)this->signals.size(); s_index++) {
				delete this->signals[s_index];
			}
			this->signals.clear();

			for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
				delete this->new_scope_histories[h_index];
			}
			this->new_scope_histories.clear();
			this->new_target_val_histories.clear();

			set_actions();
		}
	} else if (this->new_scores.size() >= CHECK_4_NUM_ITERS) {
		this->curr_explore = create_explore(this->scope_context);
		this->curr_explore->explore_node->experiment = this;

		this->positive_count = 0;
		this->true_positive_count = 0;
		this->total_count = 0;

		uniform_int_distribution<int> until_distribution(0, (int)this->scope_context->average_instances_per_run-1);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = SIGNAL_EXPERIMENT_STATE_EXPLORE;
	}
}
