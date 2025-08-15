#include "signal_experiment.h"

#include <cmath>

#include "abstract_node.h"
#include "constants.h"
#include "explore.h"
#include "globals.h"
#include "helpers.h"
#include "problem.h"

using namespace std;

const int CHECK_1_NUM_ITERS = 1;
const int CHECK_2_NUM_ITERS = 10;
const int CHECK_3_NUM_ITERS = 100;
const int CHECK_4_NUM_ITERS = 1000;

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

		if (t_score < -0.674) {
			is_fail = true;
		}
	}

	if (is_fail) {
		this->pre_actions.clear();
		this->post_actions.clear();

		this->new_scores.clear();

		geometric_distribution<int> num_actions_distribution(0.2);
		uniform_int_distribution<int> action_distribution(0, 2);
		int num_pre = num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_pre; a_index++) {
			this->pre_actions.push_back(action_distribution(generator));
		}
		int num_post = 5 + num_actions_distribution(generator);
		for (int a_index = 0; a_index < num_post; a_index++) {
			this->post_actions.push_back(action_distribution(generator));
		}
	} else if (this->new_scores.size() >= CHECK_4_NUM_ITERS) {
		this->curr_explore = create_explore(this->scope_context);
		this->curr_explore->explore_node->experiment = this;

		this->state = SIGNAL_EXPERIMENT_STATE_EXPLORE;
	}
}
