#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_new_check_activate(vector<double>& obs,
												 ExploreExperimentHistory* history,
												 SolutionWrapper* wrapper) {
	if (wrapper->should_explore
			&& wrapper->diversity_index == this->diversity_index) {
		this->num_instances_until_target--;
		if (this->num_instances_until_target <= 0) {
			history->obs_histories.push_back(obs);

			double average_instances_per_hit;
			switch (this->node_context->type) {
			case NODE_TYPE_NOOP:
				{
					NoopNode* noop_node = (NoopNode*)this->node_context;
					average_instances_per_hit = noop_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					average_instances_per_hit = action_node->average_instances_per_hit;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					average_instances_per_hit = scope_node->average_instances_per_hit;
				}
				break;
			default:
			// case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						average_instances_per_hit = branch_node->branch_average_instances_per_hit;
					} else {
						average_instances_per_hit = branch_node->original_average_instances_per_hit;
					}
				}
				break;
			}
			uniform_int_distribution<int> until_distribution(1, average_instances_per_hit);
			this->num_instances_until_target = until_distribution(generator);

			wrapper->has_explore = true;

			ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void ExploreExperiment::train_new_step(vector<double>& obs,
									   int& action,
									   bool& is_next,
									   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

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

bool ExploreExperiment::train_new_helper(int l_index) {
	if (this->new_network != NULL) {
		delete this->new_network;
	}
	this->new_network = new Network(this->new_obs_histories[0].size());
	double hidden_1_average_max_update = 0.0;
	double hidden_2_average_max_update = 0.0;
	double hidden_3_average_max_update = 0.0;
	double output_average_max_update = 0.0;

	uniform_int_distribution<int> new_train_distribution(0, this->new_obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_NEW_ITERS[l_index]; iter_index++) {
		int rand_index = new_train_distribution(generator);

		this->new_network->activate(this->new_obs_histories[rand_index]);

		double error = this->new_target_val_histories[rand_index] - this->new_network->output->acti_vals[0];

		this->new_network->init_backprop(error,
										 hidden_1_average_max_update,
										 hidden_2_average_max_update,
										 hidden_3_average_max_update,
										 output_average_max_update);
	}

	double existing_sum_vals = 0.0;
	int existing_count = 0;
	for (int h_index = 0; h_index < (int)this->existing_obs_histories.size(); h_index++) {
		this->existing_network->activate(this->existing_obs_histories[h_index]);
		double existing_predicted = this->existing_network->output->acti_vals[0];
		this->new_network->activate(this->existing_obs_histories[h_index]);
		double new_predicted = this->new_network->output->acti_vals[0];
		if (new_predicted >= existing_predicted) {
			existing_sum_vals += this->existing_target_val_histories[h_index];
			existing_count++;
		}
	}
	double existing_average = existing_sum_vals / (double)existing_count;
	double new_sum_vals = 0.0;
	int new_count = 0;
	for (int h_index = 0; h_index < (int)this->new_obs_histories.size(); h_index++) {
		this->existing_network->activate(this->new_obs_histories[h_index]);
		double existing_predicted = this->existing_network->output->acti_vals[0];
		this->new_network->activate(this->new_obs_histories[h_index]);
		double new_predicted = this->new_network->output->acti_vals[0];
		if (new_predicted >= existing_predicted) {
			new_sum_vals += this->new_target_val_histories[h_index];
			new_count++;
		}
	}
	double new_average = new_sum_vals / (double)new_count;
	double average_ratio = (existing_count + new_count)
		/ ((double)this->existing_obs_histories.size()
			+ (double)this->new_obs_histories.size());
	double local_improvement = (new_average - existing_average) * average_ratio;

	double average_instances_per_run;
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			average_instances_per_run = noop_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			average_instances_per_run = action_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			average_instances_per_run = scope_node->average_instances_per_run;
		}
		break;
	default:
	// case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				average_instances_per_run = branch_node->branch_average_instances_per_run;
			} else {
				average_instances_per_run = branch_node->original_average_instances_per_run;
			}
		}
		break;
	}
	double global_improvement = average_instances_per_run * local_improvement;

	// // temp
	// cout << "local_improvement: " << local_improvement << endl;
	// cout << "global_improvement: " << global_improvement << endl;

	bool is_success = false;
	if (local_improvement > 0.0) {
		if (this->scope_context->last_scores[l_index].size() >= MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = this->scope_context->last_scores[l_index].begin();
					it != this->scope_context->last_scores[l_index].end(); it++) {
				if (global_improvement >= *it) {
					num_better_than++;
				}
			}

			double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->last_scores[l_index].size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			}

			if (this->scope_context->last_scores[l_index].size() >= NUM_LAST_TRACK) {
				this->scope_context->last_scores[l_index].pop_front();
			}
			this->scope_context->last_scores[l_index].push_back(global_improvement);
		} else {
			this->scope_context->last_scores[l_index].push_back(global_improvement);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	return is_success || rand()%4 != 0;
	#else
	return is_success;
	#endif /* MDEBUG */
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->should_explore
			&& wrapper->diversity_index == this->diversity_index) {
		if (history->obs_histories.size() > 0) {
			for (int i_index = 0; i_index < (int)history->obs_histories.size(); i_index++) {
				this->new_obs_histories.push_back(history->obs_histories[i_index]);
				this->new_target_val_histories.push_back(target_val);
			}

			wrapper->new_since_update++;

			this->state_iter++;

			bool is_fail = false;
			for (int l_index = 0; l_index < (int)TRAIN_NEW_NUM_DATAPOINTS.size(); l_index++) {
				if (this->state_iter == TRAIN_NEW_NUM_DATAPOINTS[l_index]) {
					bool is_success = train_new_helper(l_index);
					if (!is_success) {
						is_fail = true;
					}
				}
			}

			if (is_fail) {
				this->try_iter++;
				if (this->try_iter >= EXPERIMENT_MAX_TRIES) {
					delete this;
				} else {
					this->new_obs_histories.clear();
					this->new_target_val_histories.clear();

					double average_instances_per_hit;
					switch (this->node_context->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)this->node_context;
							average_instances_per_hit = noop_node->average_instances_per_hit;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							average_instances_per_hit = action_node->average_instances_per_hit;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;
							average_instances_per_hit = scope_node->average_instances_per_hit;
						}
						break;
					default:
					// case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								average_instances_per_hit = branch_node->branch_average_instances_per_hit;
							} else {
								average_instances_per_hit = branch_node->original_average_instances_per_hit;
							}
						}
						break;
					}
					uniform_int_distribution<int> until_distribution(1, 2 * average_instances_per_hit);
					this->num_instances_until_target = until_distribution(generator);

					this->state = EXPLORE_EXPERIMENT_STATE_EXPLORE;
					this->state_iter = 0;
				}
			} else if (this->state_iter == TRAIN_NEW_NUM_DATAPOINTS.back()) {
				this->sum_vals = 0.0;

				this->state = EXPLORE_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			}
		}
	}
}
