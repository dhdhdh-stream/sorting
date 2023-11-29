#include "helpers.h"

#include "action_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment_helper(vector<int>& scope_context,
							  vector<int>& node_context,
							  vector<AbstractNode*>& possible_nodes,
							  vector<vector<int>>& possible_scope_contexts,
							  vector<vector<int>>& possible_node_contexts,
							  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				if (action_node->experiment == NULL) {
					node_context.back() = action_node->id;

					possible_nodes.push_back(action_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = -1;
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				create_experiment_helper(scope_context,
										 node_context,
										 possible_nodes,
										 possible_scope_contexts,
										 possible_node_contexts,
										 scope_node_history->inner_scope_history);

				if (scope_node->experiment == NULL) {
					possible_nodes.push_back(scope_node);
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
				}

				node_context.back() = -1;
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_branch_experiment(ScopeHistory* root_history) {
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;

	vector<int> scope_context;
	vector<int> node_context;
	create_experiment_helper(scope_context,
							 node_context,
							 possible_nodes,
							 possible_scope_contexts,
							 possible_node_contexts,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_nodes.size()-1);
	int rand_index = possible_distribution(generator);

	uniform_int_distribution<int> next_distribution(0, 1);
	int context_size = 1;
	while (true) {
		if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			context_size++;
		} else {
			break;
		}
	}

	BranchExperiment* new_branch_experiment = new BranchExperiment(
		vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
		vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

	if (possible_nodes[rand_index]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
		action_node->experiment = new_branch_experiment;
	} else {
		ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
		scope_node->experiment = new_branch_experiment;
	}
}

void create_pass_through_experiment(ScopeHistory* root_history) {
	vector<AbstractNode*> possible_nodes;
	vector<vector<int>> possible_scope_contexts;
	vector<vector<int>> possible_node_contexts;

	vector<int> scope_context;
	vector<int> node_context;
	create_experiment_helper(scope_context,
							 node_context,
							 possible_nodes,
							 possible_scope_contexts,
							 possible_node_contexts,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_nodes.size()-1);
	int rand_index = possible_distribution(generator);

	uniform_int_distribution<int> next_distribution(0, 1);
	int context_size = 1;
	while (true) {
		if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
			context_size++;
		} else {
			break;
		}
	}

	PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
		vector<int>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
		vector<int>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()));

	if (possible_nodes[rand_index]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)possible_nodes[rand_index];
		action_node->experiment = new_pass_through_experiment;
	} else {
		ScopeNode* scope_node = (ScopeNode*)possible_nodes[rand_index];
		scope_node->experiment = new_pass_through_experiment;
	}
}

void create_loop_experiment_helper(int curr_depth,
								   vector<Scope*>& scope_context,
								   vector<AbstractNode*>& node_context,
								   vector<int>& target_scope_context,
								   vector<int>& target_node_context,
								   vector<vector<Scope*>>& possible_scope_contexts,
								   vector<vector<AbstractNode*>>& possible_node_contexts,
								   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				if (action_node->id == target_node_context[curr_depth]) {
					// curr_depth == (int)target_scope_context.size()-1
					return;
				}

				node_context.back() = NULL;
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				if (scope_node->id == target_node_context[curr_depth]) {
					if (curr_depth+1 < (int)target_scope_context.size()) {
						create_loop_experiment_helper(curr_depth+1,
													  scope_context,
													  node_context,
													  target_scope_context,
													  target_node_context,
													  possible_scope_contexts,
													  possible_node_contexts,
													  scope_node_history->inner_scope_history);
					} else {
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
					}

					/**
					 * - simply return without worrying about cleanup
					 */
					return;
				} else {
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);
				}

				node_context.back() = NULL;
			}
		}
	}
	/**
	 * - don't worry about matching iter_index
	 *   - would need to track iter_index across all contexts
	 *   - minor impact anyways
	 *   - possible for all paths to be represented anyways depending on branching
	 */

	// no need to pop_back context
}

void create_loop_experiment(ScopeHistory* root_history) {
	AbstractNode* experiment_node;
	vector<int> experiment_scope_context;
	vector<int> experiment_node_context;
	{
		vector<AbstractNode*> possible_nodes;
		vector<vector<int>> possible_scope_contexts;
		vector<vector<int>> possible_node_contexts;

		vector<int> scope_context;
		vector<int> node_context;
		create_experiment_helper(scope_context,
								 node_context,
								 possible_nodes,
								 possible_scope_contexts,
								 possible_node_contexts,
								 root_history);

		uniform_int_distribution<int> possible_distribution(0, (int)possible_nodes.size()-1);
		int rand_index = possible_distribution(generator);

		experiment_node = possible_nodes[rand_index];
		experiment_scope_context = possible_scope_contexts[rand_index];
		experiment_node_context = possible_node_contexts[rand_index];
	}

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_loop_experiment_helper(0,
								  scope_context,
								  node_context,
								  experiment_scope_context,
								  experiment_node_context,
								  possible_scope_contexts,
								  possible_node_contexts,
								  root_history);

	geometric_distribution<int> length_distribution(0.2);
	int loop_length = 1 + length_distribution(generator);
	if (loop_length > (int)possible_scope_contexts.size()) {
		loop_length = (int)possible_scope_contexts.size();
	}
	int start_index = (int)possible_scope_contexts.size() - loop_length;

	uniform_int_distribution<int> next_distribution(0, 1);
	int context_size = 1 + ((int)possible_scope_contexts.back().size() - (int)possible_scope_contexts[start_index].size());
	while (true) {
		if (context_size < (int)possible_scope_contexts.back().size() && next_distribution(generator)) {
			context_size++;
		} else {
			break;
		}
	}

	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->num_input_states = 0;
	new_scope->num_local_states = 0;
	new_scope->node_counter = 0;
	PotentialScopeNode* new_potential_scope_node = new PotentialScopeNode();
	new_potential_scope_node->scope = new_scope;
	new_potential_scope_node->experiment_scope_depth = context_size;

	int potential_new_state_counter = 0;
	vector<map<pair<bool,int>, int>> potential_state_mappings(possible_scope_contexts.back().size());
	/**
	 * - innermost
	 *   - i.e., last updated
	 */
	vector<pair<int, pair<bool,int>>> potential_to_outer_mapping;
	{
		/**
		 * - start from furthest out to remove max unused state
		 */
		Scope* scope = possible_scope_contexts.back()[possible_scope_contexts.back().size() - context_size];
		for (int s_index = 0; s_index < scope->num_input_states; s_index++) {
			potential_state_mappings[{false, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({context_size-1, {false, s_index}});
		}
		for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
			potential_state_mappings[{true, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({context_size-1, {true, s_index}});
		}
	}
	for (int c_index = possible_scope_contexts.back().size() - context_size + 1; c_index < possible_scope_contexts.end().size(); c_index++) {
		int scope_depth = possible_scope_contexts.back().size()-1 - c_index;

		ScopeNode* previous_scope_node = (ScopeNode*)possible_node_contexts.back()[c_index-1];
		for (int i_index = 0; i_index < (int)previous_scope_node->input_types.size(); i_index++) {
			if (previous_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
				map<pair<bool,int>, int>::iterator it = potential_state_mappings[c_index-1]
					.find({previous_scope_node->input_outer_is_local[i_index], previous_scope_node->input_outer_indexes[i_index]});
				if (it != potential_state_mappings[c_index-1].end()) {
					potential_state_mappings[c_index][{false, previous_scope_node->input_inner_indexes[i_index]}] = it->second;
					potential_to_outer_mapping[it->second] = {scope_depth, {false, previous_scope_node->input_inner_indexes[i_index]}};
				}
			} else {
				potential_state_mappings[{false, previous_scope_node->input_inner_indexes[i_index]}] = potential_new_state_counter;
				potential_new_state_counter++;
				potential_to_outer_mapping.push_back({scope_depth, {false, previous_scope_node->input_inner_indexes[i_index]}});
			}
		}

		Scope* scope = possible_scope_contexts.back()[c_index];
		for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
			potential_state_mappings[{true, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({scope_depth, {true, s_index}});
		}
	}

	vector<AbstractNode*> new_nodes;
	/**
	 * - assign to -1 if not going to use
	 */
	map<int, int> potential_to_final_mapping;
	uniform_int_distribution<int> include_state_distribution(0, 1);
	for (int n_index = start_index; n_index < (int)possible_scope_contexts.size()+1; n_index++) {
		if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* original_action_node = (ActionNode*)possible_node_contexts[n_index].back();

			ActionNode* new_action_node = new ActionNode();

			new_action_node->action = original_action_node->action;

			for (int s_index = 0; s_index < (int)original_action_node->state_is_local.size(); s_index++) {
				map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
					.find({original_action_node->state_is_local[s_index], original_action_node->state_indexes[s_index]});
				if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
					map<int, int> final_it = potential_to_final_mapping.find(potential_it->second);
					if (final_it != potential_to_final_mapping.end()) {
						if (final_it->second != -1) {
							new_action_node->state_is_local.push_back(false);
							new_action_node->state_indexes.push_back(final_it->second);
							new_action_node->state_obs_indexes.push_back(original_action_node->state_obs_indexes[s_index]);
							new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
							new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
						}
					} else {
						if (include_state_distribution(generator) == 0) {
							potential_to_final_mapping[potential_it->second] = -1;
						} else {
							int new_state_index = new_scope->num_input_states;
							new_scope->num_input_states++;

							potential_to_final_mapping[potential_it->second] = new_state_index;

							new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
							new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
							new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
							if (potential_to_outer_mapping[potential_it->second].first.first) {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
							} else {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
							}
							new_potential_scope_node->input_outer_indexes.push_back(potential_to_outer_mapping[potential_it->second].first.second);
							new_potential_scope_node->input_init_vals.push_back(0.0);
							new_potential_scope_node->input_init_index_vals.push_back(0.0);

							new_action_node->state_is_local.push_back(false);
							new_action_node->state_indexes.push_back(new_state_index);
							new_action_node->state_obs_indexes.push_back(original_action_node->state_obs_indexes[s_index]);
							new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
							new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
						}
					}
				}
			}

			new_nodes.push_back(new_action_node);
		} else {
			ScopeNode* original_scope_node = (ScopeNode*)possible_node_contexts[n_index].back();

			ScopeNode* new_scope_node = new ScopeNode();

			new_scope_node->inner_scope = original_scope_node->inner_scope;

			for (int i_index = 0; i_index < (int)original_scope_node->input_types.size(); i_index++) {
				if (original_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
					map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
						.find({original_scope_node->input_outer_is_local[i_index], original_scope_node->input_outer_indexes[i_index]});
					if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
						map<int, int> final_it = potential_to_final_mapping.find(potential_it->second);
						if (final_it != potential_to_final_mapping.end()) {
							if (final_it->second != -1) {
								new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(false);
								new_scope_node->input_outer_indexes.push_back(final_it->second);
								new_scope_node->input_init_vals.push_back(0.0);
								new_scope_node->input_init_index_vals.push_back(0.0);
							}
						} else {
							if (include_state_distribution(generator) == 0) {
								potential_to_final_mapping[potential_it->second] = -1;
							} else {
								int new_state_index = new_scope->num_input_states;
								new_scope->num_input_states++;

								potential_to_final_mapping[potential_it->second] = new_state_index;

								new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
								new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
								if (potential_to_outer_mapping[potential_it->second].first.first) {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
								} else {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
								}
								new_potential_scope_node->input_outer_indexes.push_back(potential_to_outer_mapping[potential_it->second].first.second);
								new_potential_scope_node->input_init_vals.push_back(0.0);
								new_potential_scope_node->input_init_index_vals.push_back(0.0);

								new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(false);
								new_scope_node->input_outer_indexes.push_back(new_state_index);
								new_scope_node->input_init_vals.push_back(0.0);
								new_scope_node->input_init_index_vals.push_back(0.0);
							}
						}
					}
				} else {
					new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
					new_scope_node->input_outer_is_local.push_back(false);
					new_scope_node->input_outer_indexes.push_back(-1);
					new_scope_node->input_init_vals.push_back(original_scope_node->input_init_vals[i_index]);
					new_scope_node->input_init_index_vals.push_back(original_scope_node->input_init_index_vals[i_index]);
				}
			}

			for (int o_index = 0; o_index < (int)original_scope_node->output_inner_indexes.size(); o_index++) {
				map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
					.find({original_scope_node->output_outer_is_local[o_index], original_scope_node->output_outer_indexes[o_index]});
				if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
					map<int, int> final_it = potential_to_final_mapping.find(potential_it->second);
					if (final_it != potential_to_final_mapping.end()) {
						new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
						new_scope_node->output_outer_is_local.push_back(false);
						new_scope_node->output_outer_indexes.push_back(final_it->second);
					} else {
						if (include_state_distribution(generator) == 0) {
							potential_to_final_mapping[potential_it->second] = -1;
						} else {
							int new_state_index = new_scope->num_input_states;
							new_scope->num_input_states++;

							potential_to_final_mapping[potential_it->second] = new_state_index;

							new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
							new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
							new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
							if (potential_to_outer_mapping[potential_it->second].first.first) {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
							} else {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
							}
							new_potential_scope_node->input_outer_indexes.push_back(potential_to_outer_mapping[potential_it->second].first.second);
							new_potential_scope_node->input_init_vals.push_back(0.0);
							new_potential_scope_node->input_init_index_vals.push_back(0.0);

							new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
							new_scope_node->output_outer_is_local.push_back(false);
							new_scope_node->output_outer_indexes.push_back(new_state_index);
						}
					}
				}
			}

			new_nodes.push_back(new_scope_node);
		}
	}

	for (int n_index = 0; n_index < (int)new_nodes.size(); n_index++) {
		new_nodes[n_index]->parent = new_scope;
		new_nodes[n_index]->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[new_nodes[n_index]->id] = new_nodes[n_index];

		int next_node_id;
		AbstractNode* next_node;
		if (n_index == (int)new_nodes.size()-1) {
			next_node_id = -1;
			next_node = NULL;
		} else {
			next_node_id = n_index+1;
			next_node = new_nodes[n_index+1];
		}

		if (new_nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)new_nodes[n_index];
			action_node->next_node_id = next_node_id;
			action_node->next_node = next_node;
		} else {
			ScopeNode* scope_node = (ScopeNode*)new_nodes[n_index];
			scope_node->next_node_id = next_node_id;
			scope_node->next_node = next_node;
		}
	}

	LoopExperiment* new_loop_experiment = new LoopExperiment(
		vector<int>(experiment_scope_context.end() - context_size, experiment_scope_context.end()),
		vector<int>(experiment_node_context.end() - context_size, experiment_node_context.end()),
		new_potential_scope_node);

	if (experiment_node->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)experiment_node;
		action_node->experiment = new_loop_experiment;
	} else {
		ScopeNode* scope_node = (ScopeNode*)experiment_node;
		scope_node->experiment = new_loop_experiment;
	}
}
