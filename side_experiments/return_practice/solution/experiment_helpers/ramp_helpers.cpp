#include "experiment.h"

#include <iostream>
#include <limits>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "network.h"
#include "solution.h"
#include "start_node.h"
#include "utilities.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

void Experiment::ramp_activate(ExperimentRun* run) {
	ExperimentHistory* history;
	map<Experiment*, ExperimentHistory*>::iterator it =
		run->experiment_histories.find(this);
	if (it == run->experiment_histories.end()) {
		history = new ExperimentHistory(this);
		run->experiment_histories[this] = history;
	} else {
		history = it->second;
	}

	this->original_network->activate(run->state);
	this->branch_network->activate(run->state);

	bool is_branch;
	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		is_branch = true;
	} else {
		is_branch = false;
	}

	#if defined(MDEBUG) && MDEBUG
	if (run->wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	run->wrapper->curr_run_seed = xorshift(run->wrapper->curr_run_seed);
	#endif /* MDEBUG */

	if (is_branch) {
		history->hit_branch = true;

		if (history->is_on) {
			ExperimentState* new_experiment_state = new ExperimentState(this);
			new_experiment_state->step_index = 0;
			run->experiment_context = new_experiment_state;
		}
	}
}

void Experiment::ramp_step(int& action,
						   bool& is_next,
						   ExperimentRun* run) {
	ExperimentState* state = (ExperimentState*)run->experiment_context;
	if (state->step_index >= (int)this->actions.size()) {
		run->node_context = this->exit_next_node;

		delete run->experiment_context;
		run->experiment_context = NULL;
	} else {
		run->action_histories.push_back(this->actions[state->step_index]);

		action_helper(this->actions[state->step_index],
					  run->state,
					  run->wrapper);

		action = this->actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

void Experiment::ramp_backprop(double target_val,
							   ExperimentHistory* history,
							   Wrapper* wrapper) {
	this->total_count++;
	if (this->total_count >= TOTAL_MAX_ITERS) {
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				start_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				action_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					branch_node->branch_experiment = NULL;
				} else {
					branch_node->original_experiment = NULL;
				}
			}
			break;
		}
		delete this;

		return;
	}

	if (history->is_on) {
		if (history->hit_branch) {
			this->new_sum_scores += target_val;
			this->new_count++;

			this->state_iter++;
		}
	} else {
		if (history->hit_branch) {
			this->existing_sum_scores += target_val;
			this->existing_count++;

			this->state_iter++;
		}
	}

	switch (this->state) {
	case EXPERIMENT_STATE_RAMP:
		if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
			double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
			double new_score_average = this->new_sum_scores / (double)this->new_count;

			// temp
			cout << "this->curr_ramp: " << this->curr_ramp << endl;
			cout << "this->predicted_local_improvement: " << this->predicted_local_improvement << endl;
			cout << "this->predicted_global_improvement: " << this->predicted_global_improvement << endl;
			cout << "existing_score_average: " << existing_score_average << endl;
			cout << "new_score_average: " << new_score_average << endl;

			this->total_count = 0;
			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state_iter = 0;

			#if defined(MDEBUG) && MDEBUG
			if ((this->measure_status != MEASURE_STATUS_FAIL && new_score_average > existing_score_average) || rand()%3 != 0) {
			#else
			if (this->measure_status != MEASURE_STATUS_FAIL && new_score_average > existing_score_average) {
			#endif /* MDEBUG */
				this->curr_ramp++;

				if (this->curr_ramp == MEASURE_GEAR
						&& this->measure_status == MEASURE_STATUS_N_A) {
					this->starting_iter = wrapper->iter;

					this->state = EXPERIMENT_STATE_MEASURE;
				} else if (this->curr_ramp == EXPERIMENT_NUM_GEARS) {
					add(wrapper);

					switch (this->node_context->type) {
					case NODE_TYPE_START:
						{
							StartNode* start_node = (StartNode*)this->node_context;
							start_node->experiment = NULL;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							action_node->experiment = NULL;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								branch_node->branch_experiment = NULL;
							} else {
								branch_node->original_experiment = NULL;
							}
						}
						break;
					}
					delete this;
				}
			} else {
				this->curr_ramp--;
				if (this->curr_ramp < 0) {
					switch (this->node_context->type) {
					case NODE_TYPE_START:
						{
							StartNode* start_node = (StartNode*)this->node_context;
							start_node->experiment = NULL;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							action_node->experiment = NULL;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								branch_node->branch_experiment = NULL;
							} else {
								branch_node->original_experiment = NULL;
							}
						}
						break;
					}
					delete this;
				}
			}
		}
		break;
	case EXPERIMENT_STATE_MEASURE:
		if (this->state_iter >= MEASURE_STEP_NUM_ITERS) {
			double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
			double new_score_average = this->new_sum_scores / (double)this->new_count;

			int total_iters = wrapper->iter - this->starting_iter;
			if (total_iters < 0) {
				total_iters += numeric_limits<int>::max();
			}
			double average_hits_per_run = (2.0 * (double)this->new_count) / (double)total_iters;

			this->local_improvement = new_score_average - existing_score_average;
			this->global_improvement = average_hits_per_run * this->local_improvement;

			// // temp
			// cout << "new_score_average: " << new_score_average << endl;
			// cout << "existing_score_average: " << existing_score_average << endl;
			// cout << "this->local_improvement: " << this->local_improvement << endl;
			// cout << "this->global_improvement: " << this->global_improvement << endl;

			this->total_count = 0;
			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			bool is_success = false;
			#if defined(MDEBUG) && MDEBUG
			if (this->global_improvement > 0.0 || rand()%3 == 0) {
			#else
			if (this->global_improvement > 0.0) {
			#endif /* MDEBUG */
				if (wrapper->solution->ramp_last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->ramp_last_scores.begin();
							it != wrapper->solution->ramp_last_scores.end(); it++) {
						if (this->global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->ramp_last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->ramp_last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->ramp_last_scores.pop_front();
					}
					wrapper->solution->ramp_last_scores.push_back(this->global_improvement);
				} else {
					wrapper->solution->ramp_last_scores.push_back(this->global_improvement);
				}

				// temp
				is_success = true;
			}

			#if defined(MDEBUG) && MDEBUG
			if (is_success || rand()%3 != 0) {
			#else
			if (is_success) {
			#endif /* MDEBUG */
				this->measure_status = MEASURE_STATUS_SUCCESS;

				this->state = EXPERIMENT_STATE_RAMP;
				this->state_iter = 0;

				this->curr_ramp++;
			} else {
				this->measure_status = MEASURE_STATUS_FAIL;

				this->state = EXPERIMENT_STATE_RAMP;
				this->state_iter = 0;

				this->curr_ramp--;
			}
		}
		break;
	}
}
