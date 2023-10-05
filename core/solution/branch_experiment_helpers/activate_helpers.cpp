#include "branch_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

void BranchExperiment::activate(int& curr_node_id,
								Problem& problem,
								vector<ContextLayer>& context,
								int& exit_depth,
								int& exit_node_id,
								RunHelper& run_helper,
								BranchExperimentHistory* history) {
	bool matches_context = true;
	if (this->scope_context.size() > context.size()) {
		matches_context = false;
	} else {
		for (int c_index = 0; c_index < (int)this->scope_context.size()-1; c_index++) {
			if (this->scope_context[c_index] != context[context.size()-this->scope_context.size()+c_index].scope_id
					|| this->node_context[c_index] != context[context.size()-this->scope_context.size()+c_index].node_id) {
				matches_context = false;
				break;
			}
		}
	}

	if (matches_context) {
		if (run_helper.phase == RUN_PHASE_EXPLORE) {
			if (run_helper.selected_branch_experiment == this) {
				// already hooked

				run_helper.selected_branch_experiment_count++;

				if (run_helper.branch_experiment_history == NULL) {
					double target_probability;
					if (run_helper.selected_branch_experiment_count > this->average_instances_per_run) {
						target_probability = 0.5;
					} else {
						target_probability = 1.0 / (1.0 + 1.0 + (this->average_instances_per_run - run_helper.selected_branch_experiment_count));
						// already incremented
					}
					uniform_real_distribution<double> distribution(0.0, 1.0);
					if (distribution(generator) < target_probability) {
						history = new BranchExperimentHistory(this);
						run_helper.branch_experiment_history = history;
						context[context.size() - this->scope_context.size()]
							.scope_history->inner_branch_experiment_history = history;

						if (this->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
							explore_activate(curr_node_id,
											 problem,
											 context,
											 exit_depth,
											 exit_node_id,
											 run_helper,
											 history);
						} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
							train_activate(curr_node_id,
										   problem,
										   context,
										   exit_depth,
										   exit_node_id,
										   run_helper,
										   history);
						} else {
							measure_activate(curr_node_id,
											 problem,
											 context,
											 exit_depth,
											 exit_node_id,
											 run_helper);
						}
					} else {
						if (this->state != BRANCH_EXPERIMENT_STATE_EXPLORE) {
							simple_activate(curr_node_id,
											problem,
											context,
											exit_depth,
											exit_node_id,
											run_helper);
						}
					}
				} else {
					if (this->state != BRANCH_EXPERIMENT_STATE_EXPLORE) {
						simple_activate(curr_node_id,
										problem,
										context,
										exit_depth,
										exit_node_id,
										run_helper);
					}
				}
			} else if (run_helper.selected_branch_experiment == NULL) {
				map<BranchExperiment*, int>::iterator it = run_helper.experiments_seen_counts.find(this);
				if (it == run_helper.experiments_seen_counts.end()) {
					double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
					uniform_real_distribution<double> distribution(0.0, 1.0);
					if (distribution(generator) < selected_probability) {
						hook(context,
							 run_helper);

						run_helper.selected_branch_experiment = this;
						run_helper.selected_branch_experiment_count = 1;

						bool is_target;
						if (this->average_instances_per_run == 1.0) {
							// special case
							is_target = true;
						} else {
							double target_probability = 1.0 / (1.0 + this->average_instances_per_run);
							if (distribution(generator) < target_probability) {
								is_target = true;
							} else {
								is_target = false;
							}
						}
						if (is_target) {
							history = new BranchExperimentHistory(this);
							run_helper.branch_experiment_history = history;
							context[context.size() - this->scope_context.size()]
								.scope_history->inner_branch_experiment_history = history;

							if (this->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
								explore_activate(curr_node_id,
												 problem,
												 context,
												 exit_depth,
												 exit_node_id,
												 run_helper,
												 history);
							} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
								train_activate(curr_node_id,
											   problem,
											   context,
											   exit_depth,
											   exit_node_id,
											   run_helper,
											   history);
							} else {
								measure_activate(curr_node_id,
												 problem,
												 context,
												 exit_depth,
												 exit_node_id,
												 run_helper);
							}
						} else {
							simple_activate(curr_node_id,
											problem,
											context,
											exit_depth,
											exit_node_id,
											run_helper);
						}
					} else {
						run_helper.experiments_seen_counts[this] = 1;
					}
				}
			}
		} else {
			map<BranchExperiment*, int>::iterator it = run_helper.experiments_seen_counts.find(this);
			if (it == run_helper.experiments_seen_counts.end()) {
				run_helper.experiments_seen_order.push_back(this);
				run_helper.experiments_seen_counts[this] = 1;
			} else {
				it->second++;
			}
		}
	}
}

void BranchExperiment::hook_helper(vector<int>& scope_context,
								   vector<int>& node_context,
								   map<State*, StateStatus>& experiment_score_state_vals,
								   vector<int>& test_obs_indexes,
								   vector<double>& test_obs_vals,
								   ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->experiment_back_activate(scope_context,
													  node_context,
													  experiment_score_state_vals,
													  test_obs_indexes,
													  test_obs_vals,
													  action_node_history);
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				hook_helper(scope_context,
							node_context,
							experiment_score_state_vals,
							test_obs_indexes,
							test_obs_vals,
							scope_node_history->inner_scope_history);

				node_context.back() = -1;

				scope_node->experiment_back_activate(scope_context,
													 node_context,
													 experiment_score_state_vals,
													 test_obs_indexes,
													 test_obs_vals,
													 scope_node_history);
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				branch_node->experiment_back_activate(scope_context,
													  node_context,
													  experiment_score_state_vals,
													  test_obs_indexes,
													  test_obs_vals,
													  branch_node_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void BranchExperiment::hook(vector<ContextLayer>& context,
							RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_score_state_nodes[s_index].size(); n_index++) {
			if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_score_state_nodes[s_index][n_index];

				action_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				action_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				action_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				action_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			} else if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->new_score_state_nodes[s_index][n_index];

				scope_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				scope_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				scope_node->experiment_hook_score_state_obs_indexes.push_back(this->new_score_state_obs_indexes[s_index][n_index]);
				scope_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				scope_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			} else {
				BranchNode* branch_node = (BranchNode*)this->new_score_state_nodes[s_index][n_index];

				branch_node->experiment_hook_score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				branch_node->experiment_hook_score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				branch_node->experiment_hook_score_state_defs.push_back(this->new_score_states[s_index]);
				branch_node->experiment_hook_score_state_network_indexes.push_back(n_index);
			}
		}
	}

	if (this->obs_experiment != NULL) {
		this->obs_experiment->hook(this);
	}

	vector<int> scope_context;
	vector<int> node_context;
	hook_helper(scope_context,
				node_context,
				context[context.size() - this->scope_context.size()].experiment_score_state_vals,
				context[context.size() - this->scope_context.size()].scope_history->test_obs_indexes,
				context[context.size() - this->scope_context.size()].scope_history->test_obs_vals,
				context[context.size() - this->scope_context.size()].scope_history);
}

void BranchExperiment::unhook() {
	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_score_state_nodes[s_index].size(); n_index++) {
			if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_score_state_nodes[s_index][n_index];

				action_node->experiment_hook_score_state_scope_contexts.clear();
				action_node->experiment_hook_score_state_node_contexts.clear();
				action_node->experiment_hook_score_state_defs.clear();
				action_node->experiment_hook_score_state_network_indexes.clear();
			} else if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->new_score_state_nodes[s_index][n_index];

				scope_node->experiment_hook_score_state_scope_contexts.clear();
				scope_node->experiment_hook_score_state_node_contexts.clear();
				scope_node->experiment_hook_score_state_obs_indexes.clear();
				scope_node->experiment_hook_score_state_defs.clear();
				scope_node->experiment_hook_score_state_network_indexes.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->new_score_state_nodes[s_index][n_index];

				branch_node->experiment_hook_score_state_scope_contexts.clear();
				branch_node->experiment_hook_score_state_node_contexts.clear();
				branch_node->experiment_hook_score_state_defs.clear();
				branch_node->experiment_hook_score_state_network_indexes.clear();
			}
		}
	}

	if (this->obs_experiment != NULL) {
		this->obs_experiment->unhook(this);
	}
}

void BranchExperiment::backprop(double target_val,
								BranchExperimentHistory* history) {
	unhook();

	if (this->state == BRANCH_EXPERIMENT_STATE_EXPLORE) {
		explore_backprop(target_val,
						 history);
	} else if (this->state == BRANCH_EXPERIMENT_STATE_TRAIN) {
		train_backprop(target_val,
					   history);
	} else {
		measure_backprop(target_val);
	}
}
