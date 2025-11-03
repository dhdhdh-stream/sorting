#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_end_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int C1_NUM_SAMPLES = 1;
const int C2_NUM_SAMPLES = 2;
const int C3_NUM_SAMPLES = 3;
const int C4_NUM_SAMPLES = 4;

const int MAX_NUM_EXPLORES = 2;
#else
const int C1_NUM_SAMPLES = 1;
const int C2_NUM_SAMPLES = 10;
const int C3_NUM_SAMPLES = 100;
const int C4_NUM_SAMPLES = 1000;

const int MAX_NUM_EXPLORES = 100;
#endif /* MDEBUG */

void PassThroughExperiment::explore_check_activate(
		SolutionWrapper* wrapper) {
	wrapper->experiment_history->stack_traces.push_back(wrapper->scope_histories);

	PassThroughExperimentState* new_experiment_state = new PassThroughExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void PassThroughExperiment::explore_step(vector<double>& obs,
										 int& action,
										 bool& is_next,
										 bool& fetch_action,
										 SolutionWrapper* wrapper) {
	PassThroughExperimentState* experiment_state = (PassThroughExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			if (this->actions[experiment_state->step_index] == -1) {
				is_next = true;
				fetch_action = true;

				wrapper->num_actions++;
			} else {
				action = this->actions[experiment_state->step_index];
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			}
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void PassThroughExperiment::explore_set_action(int action,
											   PassThroughExperimentState* experiment_state) {
	this->actions[experiment_state->step_index] = action;

	experiment_state->step_index++;
}

void PassThroughExperiment::explore_exit_step(SolutionWrapper* wrapper,
											  PassThroughExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void PassThroughExperiment::explore_backprop(double target_val,
											 SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		double curr_sum_signals = 0.0;
		map<Scope*, pair<int,ScopeHistory*>> to_add;
		for (int s_index = 0; s_index < (int)history->stack_traces.size(); s_index++) {
			double sum_vals = target_val - wrapper->solution->curr_score;
			int sum_counts = 1;

			for (int l_index = 0; l_index < (int)history->stack_traces[s_index].size(); l_index++) {
				ScopeHistory* scope_history = history->stack_traces[s_index][l_index];
				Scope* scope = scope_history->scope;
				if (scope->consistency_network != NULL) {
					if (!scope_history->signal_initialized) {
						vector<double> inputs = scope_history->pre_obs;
						inputs.insert(inputs.end(), scope_history->post_obs.begin(), scope_history->post_obs.end());

						scope->consistency_network->activate(inputs);
						#if defined(MDEBUG) && MDEBUG
						scope_history->consistency_val = 2 * (rand()%2) - 1;
						#else
						scope_history->consistency_val = scope->consistency_network->output->acti_vals[0];
						#endif /* MDEBUG */

						scope->pre_network->activate(scope_history->pre_obs);
						scope_history->pre_val = scope->pre_network->output->acti_vals[0];

						scope->post_network->activate(inputs);
						scope_history->post_val = scope->post_network->output->acti_vals[0];

						scope_history->signal_initialized = true;
					}

					sum_vals += (scope_history->post_val - scope_history->pre_val);
					sum_counts++;

					this->sum_consistency += scope_history->consistency_val;
					this->consistency_count++;
				}

				map<Scope*, pair<int,ScopeHistory*>>::iterator it = to_add.find(scope);
				if (it == to_add.end()) {
					to_add[scope] = {1, scope_history};
				} else {
					uniform_int_distribution<int> add_distribution(0, it->second.first);
					if (add_distribution(generator) == 0) {
						it->second.second = scope_history;
					}
					it->second.first++;
				}
			}

			double average_val = sum_vals / sum_counts;
			curr_sum_signals += average_val;
		}
		this->sum_signals += curr_sum_signals / (double)history->stack_traces.size();

		if (this->state_iter < C2_NUM_SAMPLES) {
			for (map<Scope*, pair<int,ScopeHistory*>>::iterator it = to_add.begin();
				it != to_add.end(); it++) {
				Scope* scope = it->first;
				ScopeHistory* scope_history = it->second.second;
				int max_sample_per_timestamp = (TOTAL_MAX_SAMPLES + (int)scope->existing_pre_obs.size() - 1) / (int)scope->existing_pre_obs.size();
				if ((int)scope->explore_pre_obs.back().size() < max_sample_per_timestamp) {
					scope->explore_pre_obs.back().push_back(scope_history->pre_obs);
					scope->explore_post_obs.back().push_back(scope_history->post_obs);
				} else {
					uniform_int_distribution<int> distribution(0, scope->explore_pre_obs.back().size()-1);
					int index = distribution(generator);
					scope->explore_pre_obs.back()[index] = scope_history->pre_obs;
					scope->explore_post_obs.back()[index] = scope_history->post_obs;
				}
			}
		}

		this->sum_scores += target_val;

		this->state_iter++;
		bool is_eval = false;
		switch (this->state) {
		case PASS_THROUGH_EXPERIMENT_STATE_C1:
			if (this->state_iter >= C1_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_C2:
			if (this->state_iter >= C2_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_C3:
			if (this->state_iter >= C3_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case PASS_THROUGH_EXPERIMENT_STATE_C4:
			if (this->state_iter >= C4_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		}
		if (is_eval) {
			double new_score = this->sum_scores / this->state_iter;
			double new_signal = this->sum_signals / this->state_iter;
			double new_consistency = this->sum_consistency / this->consistency_count;

			#if defined(MDEBUG) && MDEBUG
			if ((new_score >= this->existing_score
					&& new_signal >= this->existing_signal
					&& new_consistency >= this->existing_consistency) || rand()%5 != 0) {
			#else
			if (new_score >= this->existing_score
					&& new_signal >= this->existing_signal
					&& new_consistency >= this->existing_consistency) {
			#endif /* MDEBUG */
				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_C1:
					this->state = PASS_THROUGH_EXPERIMENT_STATE_C2;
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_C2:
					this->state = PASS_THROUGH_EXPERIMENT_STATE_C3;
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_C3:
					this->state = PASS_THROUGH_EXPERIMENT_STATE_C4;
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_C4:
					double average_hits_per_run = (double)C4_NUM_SAMPLES / (double)this->total_count;

					this->improvement = average_hits_per_run * (new_score - this->existing_score);

					this->result = EXPERIMENT_RESULT_SUCCESS;
					break;
				}
			} else {
				this->num_explores++;
				if (this->num_explores >= MAX_NUM_EXPLORES) {
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					if (this->new_scope != NULL) {
						delete this->new_scope;
						this->new_scope = NULL;
					}
					this->step_types.clear();
					this->actions.clear();
					this->scopes.clear();

					vector<AbstractNode*> possible_exits;

					AbstractNode* starting_node;
					switch (this->node_context->type) {
					case NODE_TYPE_START:
						{
							StartNode* start_node = (StartNode*)this->node_context;
							starting_node = start_node->next_node;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							starting_node = action_node->next_node;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;
							starting_node = scope_node->next_node;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								starting_node = branch_node->branch_next_node;
							} else {
								starting_node = branch_node->original_next_node;
							}
						}
						break;
					case NODE_TYPE_BRANCH_END:
						{
							BranchEndNode* branch_end_node = (BranchEndNode*)this->node_context;
							starting_node = branch_end_node->next_node;
						}
						break;
					}

					this->scope_context->random_exit_activate(
						starting_node,
						possible_exits);

					geometric_distribution<int> exit_distribution(0.1);
					int random_index;
					while (true) {
						random_index = exit_distribution(generator);
						if (random_index < (int)possible_exits.size()) {
							break;
						}
					}
					this->exit_next_node = possible_exits[random_index];

					uniform_int_distribution<int> new_scope_distribution(0, 1);
					if (new_scope_distribution(generator) == 0) {
						this->new_scope = create_new_scope(this->node_context->parent);
					}
					if (this->new_scope != NULL) {
						this->step_types.push_back(STEP_TYPE_SCOPE);
						this->actions.push_back(-1);
						this->scopes.push_back(this->new_scope);
					} else {
						int new_num_steps;
						geometric_distribution<int> geo_distribution(0.3);
						/**
						 * - num_steps less than exit length on average to reduce solution size
						 */
						if (random_index == 0) {
							new_num_steps = 1 + geo_distribution(generator);
						} else {
							new_num_steps = geo_distribution(generator);
						}

						vector<int> possible_child_indexes;
						for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
							if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
								possible_child_indexes.push_back(c_index);
							}
						}
						uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
						for (int s_index = 0; s_index < new_num_steps; s_index++) {
							bool is_scope = false;
							if (possible_child_indexes.size() > 0) {
								if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
									uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
									if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
										is_scope = true;
									}
								} else {
									uniform_int_distribution<int> scope_distribution(0, 1);
									if (scope_distribution(generator) == 0) {
										is_scope = true;
									}
								}
							}
							if (is_scope) {
								this->step_types.push_back(STEP_TYPE_SCOPE);
								this->actions.push_back(-1);

								int child_index = possible_child_indexes[child_index_distribution(generator)];
								this->scopes.push_back(this->node_context->parent->child_scopes[child_index]);
							} else {
								this->step_types.push_back(STEP_TYPE_ACTION);

								this->actions.push_back(-1);

								this->scopes.push_back(NULL);
							}
						}
					}

					this->total_count = 0;
					this->total_sum_scores = 0.0;

					this->sum_scores = 0.0;
					this->sum_signals = 0.0;

					this->sum_consistency = 0.0;
					this->consistency_count = 0;

					this->state = PASS_THROUGH_EXPERIMENT_STATE_C1;
					this->state_iter = 0;
				}
			}
		}
	}
}
