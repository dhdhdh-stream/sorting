#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void Experiment::clean_check_activate(SolutionWrapper* wrapper) {
	wrapper->node_context.back() = this->exit_next_node;
}

void Experiment::clean_backprop(double target_val,
								SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;

	if (history->is_hit) {
		this->sum_true += target_val;
		this->hit_count++;
	}

	if (this->hit_count >= MEASURE_ITERS) {
		double new_true = this->sum_true / this->hit_count;
		this->improvement = new_true - this->existing_true;

		bool is_success = false;
		if (this->improvement >= 0.0) {
			if (wrapper->solution->last_clean_scores.size() >= MIN_NUM_LAST_CLEAN_TRACK) {
				int num_better_than = 0;
				// // temp
				// cout << "last_clean_scores:";
				for (list<double>::iterator it = wrapper->solution->last_clean_scores.begin();
						it != wrapper->solution->last_clean_scores.end(); it++) {
					// // temp
					// cout << " " << *it;
					if (improvement >= *it) {
						num_better_than++;
					}
				}
				// // temp
				// cout << endl;

				int target_better_than = LAST_CLEAN_BETTER_THAN_RATIO * (double)wrapper->solution->last_clean_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->last_clean_scores.size() >= NUM_LAST_CLEAN_TRACK) {
					wrapper->solution->last_clean_scores.pop_front();
				}
				wrapper->solution->last_clean_scores.push_back(improvement);
			} else {
				wrapper->solution->last_clean_scores.push_back(improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%10 == 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			this->clean_success = true;

			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}

			cout << "clean_success" << endl;

			cout << "this->improvement: " << this->improvement << endl;

			cout << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			uniform_int_distribution<int> val_input_distribution(0, this->existing_obs_histories.size()-1);

			this->existing_true_network = new Network(this->existing_obs_histories[0].size(),
													  NETWORK_SIZE_SMALL);
			for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
				int rand_index = val_input_distribution(generator);

				this->existing_true_network->activate(this->existing_obs_histories[rand_index]);

				double error = this->existing_true_histories[rand_index] - this->existing_true_network->output->acti_vals[0];

				this->existing_true_network->backprop(error);
			}

			this->clean_success = false;

			this->best_surprise = numeric_limits<double>::lowest();

			uniform_int_distribution<int> until_distribution(1, 2 * this->average_instances_per_run);
			this->num_instances_until_target = until_distribution(generator);

			this->state = EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		}
	}
}
