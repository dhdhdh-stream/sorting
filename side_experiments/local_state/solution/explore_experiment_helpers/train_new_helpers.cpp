#include "explore_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void ExploreExperiment::train_new_check_activate(vector<double>& obs,
												 ExploreExperimentHistory* history,
												 SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		this->num_instances_until_target--;
		if (this->num_instances_until_target <= 0) {
			ScopeHistory* scope_history = wrapper->scope_histories.back();

			vector<bool> curr_dependencies_is_hit(this->dependencies.size());
			vector<Eigen::VectorXf> curr_dependencies_state(this->dependencies.size());
			vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				bool is_hit;
				Eigen::VectorXf state;
				vector<double> obs;
				fetch_dependency_helper(scope_history,
										this->dependencies[d_index],
										0,
										is_hit,
										state,
										obs);
				curr_dependencies_is_hit[d_index] = is_hit;
				curr_dependencies_state[d_index] = state;
				curr_dependencies_obs[d_index] = obs;
			}
			history->dependencies_is_hit_histories.push_back(curr_dependencies_is_hit);
			history->dependencies_state_histories.push_back(curr_dependencies_state);
			history->dependencies_obs_histories.push_back(curr_dependencies_obs);
			history->state_histories.push_back(wrapper->states.back());

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

			history->signal_histories.push_back(0.0);
			wrapper->scope_histories.back()->experiment_callback_histories.push_back(history);
			wrapper->scope_histories.back()->experiment_callback_indexes.push_back(history->signal_histories.size()-1);

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

			wrapper->run_num_actions++;
		} else {
			Scope* inner_scope = this->best_scopes[experiment_state->step_index];
			ScopeHistory* inner_scope_history = new ScopeHistory(inner_scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(inner_scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(inner_scope->num_states);
			wrapper->states.back().setConstant(0.0);
			wrapper->partial_states.push_back(Eigen::VectorXf());
			wrapper->partial_states.back().resize(inner_scope->num_states);
			wrapper->partial_states.back().setConstant(0.0);

			inner_scope->experiment_start_activate(
				obs,
				wrapper);
		}
	}
}

void ExploreExperiment::train_new_callback(vector<double>& obs,
										   SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	int action = this->best_actions[experiment_state->step_index];
	ActionNode* generic_action_node = this->scope_context->generic_action_nodes[action];

	generic_action_node->action_network->activate(wrapper->states.back());

	generic_action_node->obs_network->activate(wrapper->states.back(),
											   obs);

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_exit_step(SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();
	wrapper->partial_states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::train_new_backprop(
		double target_val,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	if (wrapper->run_type == RUN_TYPE_EXPLORE) {
		if (history->dependencies_is_hit_histories.size() > 0) {
			for (int i_index = 0; i_index < (int)history->dependencies_is_hit_histories.size(); i_index++) {
				this->new_dependencies_is_hit_histories.push_back(history->dependencies_is_hit_histories[i_index]);
				this->new_dependencies_state_histories.push_back(history->dependencies_state_histories[i_index]);
				this->new_dependencies_obs_histories.push_back(history->dependencies_obs_histories[i_index]);
				this->new_state_histories.push_back(history->state_histories[i_index]);
				this->new_signal_histories.push_back(history->signal_histories[i_index]);
				this->new_target_val_histories.push_back(target_val);
			}

			this->state_iter++;
			if (this->state_iter >= EXPERIMENT_NUM_DATAPOINTS) {
				int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_dependencies_is_hit_histories.size();

				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_dependencies_is_hit_histories.begin(), this->new_dependencies_is_hit_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_dependencies_state_histories.begin(), this->new_dependencies_state_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_dependencies_obs_histories.begin(), this->new_dependencies_obs_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_state_histories.begin(), this->new_state_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_signal_histories.begin(), this->new_signal_histories.end(), generator_copy);
				}
				{
					default_random_engine generator_copy = generator;
					shuffle(this->new_target_val_histories.begin(), this->new_target_val_histories.end(), generator_copy);
				}

				int num_new_train = (1.0 - VERIFY_RATIO) * (double)this->new_dependencies_is_hit_histories.size();

				ScoreNetwork* new_network = new ScoreNetwork(this->scope_context->num_states);

				uniform_int_distribution<int> new_train_distribution(0, num_new_train-1);
				for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
					int rand_index = new_train_distribution(generator);

					new_network->activate(this->new_state_histories[rand_index]);

					if (this->use_signal) {
						new_network->init_backprop(this->new_signal_histories[rand_index]);
					} else {
						new_network->init_backprop(this->new_target_val_histories[rand_index]);
					}

					if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
						new_network->init_update();
					}
				}
				for (int s_index = 0; s_index < (int)new_network->state_input->errors.size(); s_index++) {
					new_network->state_input->errors(s_index) = 0.0;
				}

				double existing_sum_vals = 0.0;
				int existing_count = 0;
				for (int h_index = num_existing_train; h_index < (int)this->existing_dependencies_is_hit_histories.size(); h_index++) {
					this->existing_network->activate(this->existing_state_histories[h_index]);
					new_network->activate(this->existing_state_histories[h_index]);

					// // temp
					// if (h_index < num_existing_train + 10) {
					// 	cout << h_index << endl;
					// 	cout << "this->existing_state_histories[h_index]:";
					// 	for (int s_index = 0; s_index < (int)this->existing_state_histories[h_index].size(); s_index++) {
					// 		cout << " " << this->existing_state_histories[h_index][s_index];
					// 	}
					// 	cout << endl;
					// 	cout << "this->existing_network->output->acti_vals(0): " << this->existing_network->output->acti_vals(0) << endl;
					// 	cout << "new_network->output->acti_vals(0): " << new_network->output->acti_vals(0) << endl;
					// }

					if (new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
						existing_sum_vals += this->existing_target_val_histories[h_index];
						existing_count++;
					}
				}
				double existing_average = existing_sum_vals / (double)existing_count;
				double new_sum_vals = 0.0;
				int new_count = 0;
				for (int h_index = num_new_train; h_index < (int)this->new_dependencies_is_hit_histories.size(); h_index++) {
					this->existing_network->activate(this->new_state_histories[h_index]);
					new_network->activate(this->new_state_histories[h_index]);

					// // temp
					// if (h_index < num_new_train + 10) {
					// 	cout << h_index << endl;
					// 	cout << "this->new_state_histories[h_index]:";
					// 	for (int s_index = 0; s_index < (int)this->new_state_histories[h_index].size(); s_index++) {
					// 		cout << " " << this->new_state_histories[h_index][s_index];
					// 	}
					// 	cout << endl;
					// 	cout << "this->existing_network->output->acti_vals(0): " << this->existing_network->output->acti_vals(0) << endl;
					// 	cout << "new_network->output->acti_vals(0): " << new_network->output->acti_vals(0) << endl;
					// }

					if (new_network->output->acti_vals[0] >= this->existing_network->output->acti_vals[0]) {
						new_sum_vals += this->new_target_val_histories[h_index];
						new_count++;
					}
				}
				double new_average = new_sum_vals / (double)new_count;
				double average_ratio = (existing_count + new_count)
					/ ((double)this->existing_dependencies_is_hit_histories.size() - num_existing_train
						+ (double)this->new_dependencies_is_hit_histories.size() - num_new_train);
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
				// cout << "train_new" << endl;
				// cout << "this->scope_context->id: " << this->scope_context->id << endl;
				// cout << "local_improvement: " << local_improvement << endl;
				// cout << "global_improvement: " << global_improvement << endl;

				if (local_improvement > 0.0) {
					bool is_success = false;
					if (this->scope_context->reuse_last_scores.size() >= REUSE_MIN_NUM_LAST_TRACK) {
						int num_better_than = 0;
						for (list<double>::iterator it = this->scope_context->reuse_last_scores.begin();
								it != this->scope_context->reuse_last_scores.end(); it++) {
							if (global_improvement >= *it) {
								num_better_than++;
							}
						}

						double target_better_than = REUSE_LAST_BETTER_THAN_RATIO * (double)this->scope_context->reuse_last_scores.size();

						if (num_better_than >= target_better_than) {
							is_success = true;
						}

						if (this->scope_context->reuse_last_scores.size() >= REUSE_NUM_LAST_TRACK) {
							this->scope_context->reuse_last_scores.pop_front();
						}
						this->scope_context->reuse_last_scores.push_back(global_improvement);
					} else {
						this->scope_context->reuse_last_scores.push_back(global_improvement);
					}

					#if defined(MDEBUG) && MDEBUG
					if (is_success || rand()%3 != 0) {
					#else
					if (is_success) {
					#endif /* MDEBUG */
						add(false,
							new_network,
							wrapper);

						delete this;

						wrapper->experiment_iter++;
						if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
							for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
								Scope* scope = wrapper->solution->scopes[s_index];
								for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
										it != scope->nodes.end(); it++) {
									switch (it->second->type) {
									case NODE_TYPE_NOOP:
										{
											NoopNode* noop_node = (NoopNode*)it->second;
											if (noop_node->experiment != NULL) {
												delete noop_node->experiment;
											}
										}
										break;
									case NODE_TYPE_ACTION:
										{
											ActionNode* action_node = (ActionNode*)it->second;
											if (action_node->experiment != NULL) {
												delete action_node->experiment;
											}
										}
										break;
									case NODE_TYPE_SCOPE:
										{
											ScopeNode* scope_node = (ScopeNode*)it->second;
											if (scope_node->experiment != NULL) {
												delete scope_node->experiment;
											}
										}
										break;
									case NODE_TYPE_BRANCH:
										{
											BranchNode* branch_node = (BranchNode*)it->second;
											if (branch_node->original_experiment != NULL) {
												delete branch_node->original_experiment;
											}
											if (branch_node->branch_experiment != NULL) {
												delete branch_node->branch_experiment;
											}
										}
										break;
									}
								}
							}

							wrapper->experiment_iter = 0;
						}
					} else {
						delete new_network;
					}
				} else {
					delete new_network;

					new_state_helper(wrapper);
				}
			}
		}
	}
}
