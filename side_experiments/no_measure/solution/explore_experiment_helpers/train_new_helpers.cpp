#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_new_check_activate(
		SolutionWrapper* wrapper) {
	ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();
	ExploreExperimentHistory* history = wrapper->explore_experiment_histories[this];

	if (experiment_state->step_index == 0) {
		if (wrapper->should_explore) {
			this->num_instances_until_target--;
			if (this->num_instances_until_target <= 0) {
				history->obs_histories.push_back(obs);

				uniform_int_distribution<int> until_distribution(1, this->average_instances_per_run);
				this->num_instances_until_target = until_distribution(generator);
			} else {
				delete experiment_state;
				wrapper->experiment_context.back() = NULL;
				return;
			}
		} else {
			history->obs_histories.push_back(obs);

			delete experiment_state;
			wrapper->experiment_context.back() = NULL;
			return;
		}
	}

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->best_scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void ExploreExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		if (history->obs_histories.size() > 0) {
			for (int i_index = 0; i_index < (int)history->obs_histories.size(); i_index++) {
				this->new_obs_histories.push_back(history->obs_histories[i_index]);
				this->new_target_val_histories.push_back(target_val);
			}

			this->state_iter++;
			if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
				{
					default_random_engine generator_copy = generator;
					shuffle(this->existing_obs_histories.begin(), this->existing_obs_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->existing_target_val_histories.begin(), this->existing_target_val_histories.end(), generator_copy);
				}

				int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_obs_histories.size();

				uniform_int_distribution<int> existing_train_distribution(0, num_existing_train-1);

				for (int iter_index = 0; iter_index < UPDATE_ITERS; iter_index++) {
					int rand_index = existing_train_distribution(generator);

					this->existing_network->activate(this->existing_obs_histories[rand_index]);

					double error = this->existing_target_val_histories[rand_index] - this->existing_network->output->acti_vals[0];

					this->existing_network->backprop(error);
					this->existing_network->update();
				}

				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_obs_histories.begin(), this->new_obs_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_target_val_histories.begin(), this->new_target_val_histories.end(), generator_copy);
				}

				int num_new_train = (1.0 - VERIFY_RATIO) * (double)this->new_obs_histories.size();

				uniform_int_distribution<int> new_train_distribution(0, num_new_train-1);

				this->new_network = new Network(this->new_obs_histories[0].size());
				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = new_train_distribution(generator);

					this->new_network->activate(this->new_obs_histories[rand_index]);

					double error = this->new_target_val_histories[rand_index] - this->new_network->output->acti_vals[0];

					this->new_network->backprop(error);
					this->new_network->update();
				}

				double existing_sum_vals = 0.0;
				int existing_count = 0;
				for (int h_index = num_existing_train; h_index < (int)this->existing_obs_histories.size(); h_index++) {
					this->existing_network->activate(this->existing_obs_histories[h_index]);
					this->new_network->activate(this->existing_obs_histories[h_index]);
					if (this->new_network->output->acti_vals[0] >= this->existing_network->output->acti_vals[0]) {
						existing_sum_vals += this->existing_target_val_histories[h_index];
						existing_count++;
					}
				}
				double existing_average = existing_sum_vals / (double)existing_count;
				double new_sum_vals = 0.0;
				int new_count = 0;
				for (int h_index = num_new_train; h_index < (int)this->new_obs_histories.size(); h_index++) {
					this->existing_network->activate(this->new_obs_histories[h_index]);
					this->new_network->activate(this->new_obs_histories[h_index]);
					if (this->new_network->output->acti_vals[0] >= this->existing_network->output->acti_vals[0]) {
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

				// // temp
				// cout << "this->scope_context->id: " << this->scope_context->id << endl;
				// cout << "local_improvement: " << local_improvement << endl;
				// cout << "global_improvement: " << global_improvement << endl;

				bool is_success = false;
				if (local_improvement > 0.0) {
					if (this->scope_context->last_scores.size() >= MIN_NUM_LAST_TRACK) {
						int num_better_than = 0;
						for (list<double>::iterator it = this->scope_context->last_scores.begin();
								it != this->scope_context->last_scores.end(); it++) {
							if (global_improvement >= *it) {
								num_better_than++;
							}
						}

						double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->last_scores.size();

						if (num_better_than >= target_better_than) {
							is_success = true;
						}

						if (this->scope_context->last_scores.size() >= NUM_LAST_TRACK) {
							this->scope_context->last_scores.pop_front();
						}
						this->scope_context->last_scores.push_back(global_improvement);
					} else {
						this->scope_context->last_scores.push_back(global_improvement);
					}
				}

				#if defined(MDEBUG) && MDEBUG
				if (is_success || rand()%3 != 0) {
				#else
				if (is_success) {
				#endif /* MDEBUG */
					add(wrapper);
				}

				this->node_context->experiment = NULL;
				delete this;

				wrapper->experiment_iter++;
				if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
					for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
						Scope* scope = wrapper->solution->scopes[s_index];
						for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
								it != scope->nodes.end(); it++) {
							if (it->second->experiment != NULL) {
								delete it->second->experiment;
								it->second->experiment = NULL;
							}
						}
					}

					wrapper->experiment_iter = 0;
				}
			}
		}
	} else {
		for (int i_index = 0; i_index < (int)history->obs_histories.size(); i_index++) {
			this->existing_obs_histories.push_back(history->obs_histories[i_index]);
			this->existing_target_val_histories.push_back(target_val);
		}
	}
}
