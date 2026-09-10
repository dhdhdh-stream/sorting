#include "refine.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_EPOCHS = 2;
const int RUNS_PER_EPOCH = 20;
#else
const int NUM_EPOCHS = 5;
const int RUNS_PER_EPOCH = 1000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NEW_TRAIN_NUM_ITERS = 2;
const int BALANCE_TRAIN_NUM_ITERS = 2;
#else
const int NEW_TRAIN_NUM_ITERS = 10;
const int BALANCE_TRAIN_NUM_ITERS = 10;
#endif /* MDEBUG */

void Refine::experiment_check_activate(std::vector<double>& obs,
									   SolutionWrapper* wrapper) {
	map<Refine*, RefineHistory*>::iterator it =
		wrapper->refine_histories.find(this);
	if (it == wrapper->refine_histories.end()) {
		it = wrapper->refine_histories.insert({this, new RefineHistory(this)}).first;
	}

	if (it->second->is_active) {
		bool is_branch;
		this->existing_network->activate(obs);
		this->new_network->activate(obs);
		if (this->new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			is_branch = true;
		} else {
			is_branch = false;
		}

		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#endif /* MDEBUG */

		it->second->is_branch.push_back(is_branch);
		it->second->obs_histories.push_back(obs);

		if (is_branch) {
			RefineState* new_experiment_state = new RefineState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void Refine::experiment_step(std::vector<double>& obs,
							 int& action,
							 bool& is_next,
							 bool& fetch_action,
							 SolutionWrapper* wrapper) {
	RefineState* experiment_state = (RefineState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Refine::set_action(int action,
						SolutionWrapper* wrapper) {
	// not reachable
}

void Refine::experiment_exit_step(SolutionWrapper* wrapper) {
	RefineState* experiment_state = (RefineState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Refine::backprop(double target_val,
					  RefineHistory* history,
					  SolutionWrapper* wrapper) {
	if (!wrapper->should_explore
			&& history->is_active) {
		for (int i_index = 0; i_index < (int)history->is_branch.size(); i_index++) {
			if (history->is_branch[i_index]) {
				uniform_int_distribution<int> sample_distribution(0, this->new_obs_histories.size()-1);
				for (int iter_index = 0; iter_index < NEW_TRAIN_NUM_ITERS; iter_index++) {
					this->new_network->activate(history->obs_histories[i_index]);
					double error = target_val - this->new_network->output->acti_vals(0);
					this->new_network->backprop(error);

					for (int b_index = 0; b_index < BALANCE_TRAIN_NUM_ITERS; b_index++) {
						int sample_index = sample_distribution(generator);

						this->new_network->activate(this->new_obs_histories[sample_index]);
						double error = this->new_target_val_histories[sample_index] - this->new_network->output->acti_vals(0);
						this->new_network->backprop(error);
					}

					this->new_network->update();

					this->new_obs_histories.push_back(history->obs_histories[i_index]);
					this->new_target_val_histories.push_back(target_val);
				}
			} else {
				this->existing_network->activate(history->obs_histories[i_index]);
				double error = target_val - this->existing_network->output->acti_vals(0);
				this->existing_network->backprop(error);

				this->existing_network->update();
			}
		}

		this->run_iter++;
		if (this->run_iter >= RUNS_PER_EPOCH) {
			this->epoch_iter++;
			if (this->epoch_iter >= NUM_EPOCHS) {
				double existing_val_average = this->existing_sum_scores / this->existing_count;
				double new_val_average = this->new_sum_scores / this->new_count;

				double local_improvement = new_val_average - existing_val_average;

				int total_iters = wrapper->iters_since_update - this->start_iter;
				if (total_iters < 0) {
					total_iters += numeric_limits<int>::max();
				}
				double average_hits_per_run = (double)this->run_iter / (double)total_iters;

				double global_improvement = average_hits_per_run * local_improvement;

				bool is_success = false;
				if (local_improvement > 0.0) {
					if (this->scope_context->measure_last_scores.size() >= MIN_NUM_LAST_TRACK) {
						int num_better_than = 0;
						for (list<double>::iterator it = this->scope_context->measure_last_scores.begin();
								it != this->scope_context->measure_last_scores.end(); it++) {
							if (global_improvement >= *it) {
								num_better_than++;
							}
						}

						double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->measure_last_scores.size();

						if (num_better_than >= target_better_than) {
							is_success = true;
						}

						if (this->scope_context->measure_last_scores.size() >= NUM_LAST_TRACK) {
							this->scope_context->measure_last_scores.pop_front();
						}
						this->scope_context->measure_last_scores.push_back(global_improvement);
					} else {
						this->scope_context->measure_last_scores.push_back(global_improvement);
					}
				}

				#if defined(MDEBUG) && MDEBUG
				if (is_success || rand()%3 != 0) {
				#else
				if (is_success) {
				#endif /* MDEBUG */
					add(wrapper);
				}

				delete this;
			} else {
				this->start_iter = wrapper->iters_since_update;
				this->existing_sum_scores = 0.0;
				this->existing_count = 0;
				this->new_sum_scores = 0.0;
				this->new_count = 0;

				this->run_iter = 0;
			}
		}
	}
}
