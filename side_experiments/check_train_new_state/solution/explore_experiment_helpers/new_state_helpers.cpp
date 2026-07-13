#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "network.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_STATE_TRAIN_ITERS = 50;
#else
const int NEW_STATE_TRAIN_ITERS = 500000;
#endif /* MDEBUG */

const int NEW_STATE_NUM_ADD = 4;

double ExploreExperiment::new_state_helper(SolutionWrapper* wrapper) {
	// cout << "this->best_dependencies:" << endl;
	// for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
	// 	cout << d_index << ":";
	// 	for (int l_index = 0; l_index < (int)this->best_dependencies[d_index].size(); l_index++) {
	// 		cout << " " << this->best_dependencies[d_index][l_index];
	// 	}
	// 	cout << endl;
	// }

	int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_obs_histories.size();

	vector<InitNetwork*> init_networks(this->best_dependencies.size());
	for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
		init_networks[d_index] = new InitNetwork(NEW_STATE_NUM_ADD,
												 wrapper->solution->num_obs);
	}
	vector<double> hidden_1_average_max_updates(this->best_dependencies.size(), 0.0);
	vector<double> hidden_2_average_max_updates(this->best_dependencies.size(), 0.0);
	vector<double> output_average_max_updates(this->best_dependencies.size(), 0.0);
	ScoreNetwork* score_network = new ScoreNetwork(NEW_STATE_NUM_ADD);
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	int num_new_train = (1.0 - VERIFY_RATIO) * (double)this->new_dependencies_is_hit_histories.size();

	uniform_int_distribution<int> new_train_distribution(0, num_new_train-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
	// for (int iter_index = 0; iter_index < NEW_STATE_TRAIN_ITERS; iter_index++) {
		int rand_index = new_train_distribution(generator);

		vector<double> state(NEW_STATE_NUM_ADD, 0.0);

		for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
			if (this->new_dependencies_is_hit_histories[rand_index][d_index]) {
				init_networks[d_index]->init_activate(state,
													  this->new_dependencies_obs_histories[rand_index][d_index]);
			}
		}

		score_network->activate(state);

		score_network->init_backprop(this->new_target_val_histories[rand_index]);

		vector<double> state_errors(NEW_STATE_NUM_ADD, 0.0);

		for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
			state_errors[s_index] = score_network->state_input->errors(s_index);
			score_network->state_input->errors(s_index) = 0.0;
		}

		for (int d_index = (int)this->best_dependencies.size()-1; d_index >= 0; d_index--) {
			if (this->new_dependencies_is_hit_histories[rand_index][d_index]) {
				init_networks[d_index]->init_backprop(state_errors);
			}
		}

		if ((iter_index+1)%EPOCH_SIZE == 0) {
			for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
				init_networks[d_index]->init_update(hidden_1_average_max_updates[d_index],
													hidden_2_average_max_updates[d_index],
													output_average_max_updates[d_index]);
			}
			score_network->init_update(hidden_1_average_max_update,
									   hidden_2_average_max_update,
									   output_average_max_update);
		}
	}
	for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
		for (int i_index = 0; i_index < (int)init_networks[d_index]->state_input->errors.size(); i_index++) {
			init_networks[d_index]->state_input->errors(i_index) = 0.0;
		}
	}
	for (int i_index = 0; i_index < (int)score_network->state_input->errors.size(); i_index++) {
		score_network->state_input->errors(i_index) = 0.0;
	}

	// double sum_misguess = 0.0;
	// for (int h_index = 0; h_index < num_new_train; h_index++) {
	// 	vector<double> state(NEW_STATE_NUM_ADD, 0.0);
	// 	for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
	// 		if (this->new_dependencies_is_hit_histories[h_index][d_index]) {
	// 			init_networks[d_index]->init_activate(state,
	// 												  this->new_dependencies_obs_histories[h_index][d_index]);
	// 		}
	// 	}
	// 	score_network->activate(state);

	// 	sum_misguess += (this->new_target_val_histories[h_index] - score_network->output->acti_vals(0))
	// 		* (this->new_target_val_histories[h_index] - score_network->output->acti_vals(0));
	// }
	// double misguess_average = sum_misguess / num_new_train;

	double existing_sum_vals = 0.0;
	int existing_count = 0;
	for (int h_index = num_existing_train; h_index < (int)this->existing_obs_histories.size(); h_index++) {
		this->existing_network->activate(this->existing_obs_histories[h_index]);

		vector<double> state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
			if (this->existing_dependencies_is_hit_histories[h_index][d_index]) {
				init_networks[d_index]->init_activate(state,
													  this->existing_dependencies_obs_histories[h_index][d_index]);
			}
		}
		score_network->activate(state);

		// // temp
		// if (h_index < num_existing_train + 10) {
		// 	cout << "this->existing_network->output->acti_vals(0): " << this->existing_network->output->acti_vals(0) << endl;
		// 	cout << "state:";
		// 	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
		// 		cout << " " << state[s_index];
		// 	}
		// 	cout << endl;
		// 	cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
		// 	cout << "this->existing_target_val_histories[h_index]: " << this->existing_target_val_histories[h_index] << endl;
		// }

		if (score_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			existing_sum_vals += this->existing_target_val_histories[h_index];
			existing_count++;
		}
	}
	double existing_average = existing_sum_vals / (double)existing_count;
	double new_sum_vals = 0.0;
	int new_count = 0;
	for (int h_index = num_new_train; h_index < (int)this->new_obs_histories.size(); h_index++) {
		this->existing_network->activate(this->new_obs_histories[h_index]);

		vector<double> state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->best_dependencies.size(); d_index++) {
			if (this->new_dependencies_is_hit_histories[h_index][d_index]) {
				init_networks[d_index]->init_activate(state,
													  this->new_dependencies_obs_histories[h_index][d_index]);
			}
		}
		score_network->activate(state);

		// // temp
		// if (h_index < num_new_train + 10) {
		// 	cout << "this->existing_network->output->acti_vals(0): " << this->existing_network->output->acti_vals(0) << endl;
		// 	cout << "state:";
		// 	for (int s_index = 0; s_index < (int)state.size(); s_index++) {
		// 		cout << " " << state[s_index];
		// 	}
		// 	cout << endl;
		// 	cout << "score_network->output->acti_vals(0): " << score_network->output->acti_vals(0) << endl;
		// 	cout << "this->new_target_val_histories[h_index]: " << this->new_target_val_histories[h_index] << endl;
		// }

		if (score_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			new_sum_vals += this->new_target_val_histories[h_index];
			new_count++;
		}
	}
	double new_average = new_sum_vals / (double)new_count;
	double average_ratio = (existing_count + new_count)
		/ ((double)this->existing_obs_histories.size() - num_existing_train
			+ (double)this->new_obs_histories.size() - num_new_train);
	double local_improvement = (new_average - existing_average) * average_ratio;

	int total_iters = wrapper->iter - this->start_iter;
	if (total_iters < 0) {
		total_iters += numeric_limits<int>::max();
	}
	double average_instances_per_run = ((double)this->existing_obs_histories.size() + (double)this->new_obs_histories.size()) / (double)total_iters;

	double global_improvement = average_instances_per_run * local_improvement;

	// cout << "new_state" << endl;
	// cout << "misguess_average: " << misguess_average << endl;
	// cout << "existing_average: " << existing_average << endl;
	// cout << "existing_count: " << existing_count << endl;
	// cout << "new_average: " << new_average << endl;
	// cout << "new_count: " << new_count << endl;
	// cout << "local_improvement: " << local_improvement << endl;
	// cout << "global_improvement: " << global_improvement << endl;

	for (int n_index = 0; n_index < (int)init_networks.size(); n_index++) {
		delete init_networks[n_index];
	}
	delete score_network;

	return global_improvement;
}
