#include "explore_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "network.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NUM_MEASURE_ITERS = 20;
#else
const int NUM_MEASURE_ITERS = 4000;
#endif /* MDEBUG */

void ExploreExperiment::measure_check_activate(
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::measure_step(vector<double>& obs,
									 int& action,
									 bool& is_next,
									 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index == 0) {
		bool is_branch = true;
		this->existing_network->activate(obs);
		this->new_network->activate(obs);
		if (this->new_network->output->acti_vals[0] >= this->existing_network->output->acti_vals[0]) {
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

		if (!is_branch) {
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

void ExploreExperiment::measure_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::measure_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore) {
		this->sum_new_scores += target_val;
		this->new_count++;

		if (this->new_count >= NUM_MEASURE_ITERS) {
			double existing_average = this->sum_existing_scores / this->existing_count;
			double new_average = this->sum_new_scores / this->new_count;

			double local_improvement = new_average - existing_average;

			int total_iters = wrapper->iter - this->start_iter;
			if (total_iters < 0) {
				total_iters += numeric_limits<int>::max();
			}
			double average_hits_per_run = (this->existing_count + this->new_count) / (double)total_iters;

			double global_improvement = average_hits_per_run * local_improvement;

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
	} else {
		this->sum_existing_scores += target_val;
		this->existing_count++;
	}
}
