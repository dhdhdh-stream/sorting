#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

/**
 * - if loop, simply include every node
 * 
 * - special case last node outside
 */
void create_loop_experiment_helper(int target_depth,
								   vector<Scope*>& scope_context,
								   vector<AbstractNode*>& node_context,
								   vector<vector<Scope*>>& possible_scope_contexts,
								   vector<vector<AbstractNode*>>& possible_node_contexts,
								   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			AbstractNodeHistory* node_history = scope_history->node_histories[i_index][h_index];
			if (node_history->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			} else if (node_history->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				if (i_index == (int)scope_history->node_histories.size()-1
						&& h_index == (int)scope_history->node_histories[i_index].size()-1
						&& (int)scope_context.size()+1 <= target_depth) {
					create_loop_experiment_helper(target_depth,
												  scope_context,
												  node_context,
												  possible_scope_contexts,
												  possible_node_contexts,
												  scope_node_history->inner_scope_history);
				} else {
					possible_scope_contexts.push_back(scope_context);
					possible_node_contexts.push_back(node_context);

					node_context.back() = NULL;
				}
			}
		}
	}

	// no need to pop_back context
}

PotentialScopeNode* create_loop(vector<ContextLayer>& context,
								int explore_context_depth) {
	ScopeHistory* scope_history = context[context.size() - explore_context_depth].scope_history;

	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_loop_experiment_helper(explore_context_depth,
								  scope_context,
								  node_context,
								  possible_scope_contexts,
								  possible_node_contexts,
								  scope_history);

	geometric_distribution<int> length_distribution(0.2);
	int loop_length = 1 + length_distribution(generator);
	if (loop_length > (int)possible_scope_contexts.size()) {
		loop_length = (int)possible_scope_contexts.size();
	}
	int start_index = (int)possible_scope_contexts.size() - loop_length;

	Scope* new_scope = new Scope();
	// don't set id/increment scope_counter until train
	new_scope->num_input_states = 0;
	new_scope->num_local_states = 0;
	new_scope->node_counter = 0;
	PotentialScopeNode* new_potential_scope_node = new PotentialScopeNode();
	new_potential_scope_node->scope = new_scope;
	new_potential_scope_node->experiment_scope_depth = explore_context_depth;

	int potential_new_state_counter = 0;
	vector<map<pair<bool,int>, int>> potential_state_mappings(explore_context_depth);
	/**
	 * - innermost
	 *   - i.e., last updated
	 */
	vector<pair<int, pair<bool,int>>> potential_to_outer_mapping;
	vector<int> potential_innermost_state_ids;
	{
		/**
		 * - start from furthest out to remove max unused state
		 */
		Scope* scope = possible_scope_contexts.back()[0];
		for (int s_index = 0; s_index < scope->num_input_states; s_index++) {
			potential_state_mappings[0][{false, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({explore_context_depth-1, {false, s_index}});
			potential_innermost_state_ids.push_back(scope->original_input_state_ids[s_index]);
		}
		for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
			potential_state_mappings[0][{true, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({explore_context_depth-1, {true, s_index}});
			potential_innermost_state_ids.push_back(scope->original_local_state_ids[s_index]);
		}
	}
	for (int c_index = 1; c_index < explore_context_depth; c_index++) {
		int scope_depth = explore_context_depth-1 - c_index;

		Scope* scope = possible_scope_contexts.back()[c_index];

		ScopeNode* previous_scope_node = (ScopeNode*)possible_node_contexts.back()[c_index-1];
		for (int i_index = 0; i_index < (int)previous_scope_node->input_types.size(); i_index++) {
			if (previous_scope_node->input_types[i_index] == INPUT_TYPE_STATE) {
				map<pair<bool,int>, int>::iterator it = potential_state_mappings[c_index-1]
					.find({previous_scope_node->input_outer_is_local[i_index], previous_scope_node->input_outer_indexes[i_index]});
				if (it != potential_state_mappings[c_index-1].end()) {
					potential_state_mappings[c_index][{previous_scope_node->input_inner_is_local[i_index], previous_scope_node->input_inner_indexes[i_index]}] = it->second;
					potential_to_outer_mapping[it->second] = {scope_depth, {previous_scope_node->input_inner_is_local[i_index], previous_scope_node->input_inner_indexes[i_index]}};
					if (previous_scope_node->input_inner_is_local[i_index]) {
						potential_innermost_state_ids[it->second] = scope->original_local_state_ids[previous_scope_node->input_inner_indexes[i_index]];
					} else {
						potential_innermost_state_ids[it->second] = scope->original_input_state_ids[previous_scope_node->input_inner_indexes[i_index]];
					}
				}
			}
			// else inner_is_local, and let inner handle
		}

		for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
			potential_state_mappings[c_index][{true, s_index}] = potential_new_state_counter;
			potential_new_state_counter++;
			potential_to_outer_mapping.push_back({scope_depth, {true, s_index}});
			potential_innermost_state_ids.push_back(scope->original_local_state_ids[s_index]);
		}
	}

	vector<AbstractNode*> new_nodes;

	ActionNode* new_noop_action_node = new ActionNode();
	new_noop_action_node->action = Action(ACTION_NOOP);
	new_nodes.push_back(new_noop_action_node);

	/**
	 * - assign to -1 if not going to use
	 */
	map<int, int> potential_to_final_mapping;
	uniform_int_distribution<int> include_state_distribution(0, 3);
	uniform_int_distribution<int> output_distribution(0, 1);
	for (int n_index = start_index; n_index < (int)possible_scope_contexts.size(); n_index++) {
		if (possible_node_contexts[n_index].back()->type == NODE_TYPE_ACTION) {
			ActionNode* original_action_node = (ActionNode*)possible_node_contexts[n_index].back();

			ActionNode* new_action_node = new ActionNode();

			new_action_node->action = original_action_node->action;

			vector<int> obs_index_mapping(original_action_node->state_is_local.size(), -1);
			for (int s_index = 0; s_index < (int)original_action_node->state_is_local.size(); s_index++) {
				if (original_action_node->state_obs_indexes[s_index] == -1) {
					map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
						.find({original_action_node->state_is_local[s_index], original_action_node->state_indexes[s_index]});
					if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
						map<int, int>::iterator final_it = potential_to_final_mapping.find(potential_it->second);
						if (final_it != potential_to_final_mapping.end()) {
							if (final_it->second != -1) {
								obs_index_mapping[s_index] = (int)new_action_node->state_is_local.size();

								new_action_node->state_is_local.push_back(true);
								new_action_node->state_indexes.push_back(final_it->second);
								new_action_node->state_obs_indexes.push_back(-1);
								new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
								new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
							}
						} else {
							if (include_state_distribution(generator) == 0) {
								potential_to_final_mapping[potential_it->second] = -1;
							} else {
								int new_state_index = new_scope->num_local_states;
								new_scope->num_local_states++;
								new_scope->original_local_state_ids.push_back(potential_innermost_state_ids[potential_it->second]);

								potential_to_final_mapping[potential_it->second] = new_state_index;

								new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
								new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
								if (potential_to_outer_mapping[potential_it->second].second.first) {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
								} else {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
								}
								new_potential_scope_node->input_outer_indexes.push_back(
									(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
								new_potential_scope_node->input_init_vals.push_back(0.0);
								new_potential_scope_node->input_init_index_vals.push_back(0.0);

								if (output_distribution(generator) == 0) {
									new_potential_scope_node->output_inner_indexes.push_back(new_state_index);
									new_potential_scope_node->output_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
									if (potential_to_outer_mapping[potential_it->second].second.first) {
										new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_LOCAL);
									} else {
										new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_INPUT);
									}
									new_potential_scope_node->output_outer_indexes.push_back(
										(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
								}

								obs_index_mapping[s_index] = (int)new_action_node->state_is_local.size();

								new_action_node->state_is_local.push_back(true);
								new_action_node->state_indexes.push_back(new_state_index);
								new_action_node->state_obs_indexes.push_back(-1);
								new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
								new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
							}
						}
					}
				} else {
					if (obs_index_mapping[original_action_node->state_obs_indexes[s_index]] != -1) {
						map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
							.find({original_action_node->state_is_local[s_index], original_action_node->state_indexes[s_index]});
						if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
							map<int, int>::iterator final_it = potential_to_final_mapping.find(potential_it->second);
							if (final_it != potential_to_final_mapping.end()) {
								if (final_it->second != -1) {
									obs_index_mapping[s_index] = (int)new_action_node->state_is_local.size();

									new_action_node->state_is_local.push_back(true);
									new_action_node->state_indexes.push_back(final_it->second);
									new_action_node->state_obs_indexes.push_back(obs_index_mapping[original_action_node->state_obs_indexes[s_index]]);
									new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
									new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
								}
							} else {
								if (include_state_distribution(generator) == 0) {
									potential_to_final_mapping[potential_it->second] = -1;
								} else {
									int new_state_index = new_scope->num_local_states;
									new_scope->num_local_states++;
									new_scope->original_local_state_ids.push_back(potential_innermost_state_ids[potential_it->second]);

									potential_to_final_mapping[potential_it->second] = new_state_index;

									new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
									new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
									new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
									if (potential_to_outer_mapping[potential_it->second].second.first) {
										new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
									} else {
										new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
									}
									new_potential_scope_node->input_outer_indexes.push_back(
										(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
									new_potential_scope_node->input_init_vals.push_back(0.0);
									new_potential_scope_node->input_init_index_vals.push_back(0.0);

									if (output_distribution(generator) == 0) {
										new_potential_scope_node->output_inner_indexes.push_back(new_state_index);
										new_potential_scope_node->output_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
										if (potential_to_outer_mapping[potential_it->second].second.first) {
											new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_LOCAL);
										} else {
											new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_INPUT);
										}
										new_potential_scope_node->output_outer_indexes.push_back(
											(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
									}

									obs_index_mapping[s_index] = (int)new_action_node->state_is_local.size();

									new_action_node->state_is_local.push_back(true);
									new_action_node->state_indexes.push_back(new_state_index);
									new_action_node->state_obs_indexes.push_back(obs_index_mapping[original_action_node->state_obs_indexes[s_index]]);
									new_action_node->state_defs.push_back(original_action_node->state_defs[s_index]);
									new_action_node->state_network_indexes.push_back(original_action_node->state_network_indexes[s_index]);
								}
							}
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
						map<int, int>::iterator final_it = potential_to_final_mapping.find(potential_it->second);
						if (final_it != potential_to_final_mapping.end()) {
							if (final_it->second != -1) {
								new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_scope_node->input_inner_is_local.push_back(original_scope_node->input_inner_is_local[i_index]);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(true);
								new_scope_node->input_outer_indexes.push_back(final_it->second);
								new_scope_node->input_init_vals.push_back(0.0);
								new_scope_node->input_init_index_vals.push_back(0.0);
							}
						} else {
							if (include_state_distribution(generator) == 0) {
								potential_to_final_mapping[potential_it->second] = -1;
							} else {
								int new_state_index = new_scope->num_local_states;
								new_scope->num_local_states++;
								new_scope->original_local_state_ids.push_back(potential_innermost_state_ids[potential_it->second]);

								potential_to_final_mapping[potential_it->second] = new_state_index;

								new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
								new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
								if (potential_to_outer_mapping[potential_it->second].second.first) {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
								} else {
									new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
								}
								new_potential_scope_node->input_outer_indexes.push_back(
									(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
								new_potential_scope_node->input_init_vals.push_back(0.0);
								new_potential_scope_node->input_init_index_vals.push_back(0.0);

								if (output_distribution(generator) == 0) {
									new_potential_scope_node->output_inner_indexes.push_back(new_state_index);
									new_potential_scope_node->output_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
									if (potential_to_outer_mapping[potential_it->second].second.first) {
										new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_LOCAL);
									} else {
										new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_INPUT);
									}
									new_potential_scope_node->output_outer_indexes.push_back(
										(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
								}

								new_scope_node->input_types.push_back(INPUT_TYPE_STATE);
								new_scope_node->input_inner_is_local.push_back(original_scope_node->input_inner_is_local[i_index]);
								new_scope_node->input_inner_indexes.push_back(original_scope_node->input_inner_indexes[i_index]);
								new_scope_node->input_outer_is_local.push_back(true);
								new_scope_node->input_outer_indexes.push_back(new_state_index);
								new_scope_node->input_init_vals.push_back(0.0);
								new_scope_node->input_init_index_vals.push_back(0.0);
							}
						}
					}
				} else {
					new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_scope_node->input_inner_is_local.push_back(original_scope_node->input_inner_is_local[i_index]);
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
					map<int, int>::iterator final_it = potential_to_final_mapping.find(potential_it->second);
					if (final_it != potential_to_final_mapping.end()) {
						if (final_it->second != -1) {
							new_scope_node->output_inner_is_local.push_back(original_scope_node->output_inner_is_local[o_index]);
							new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
							new_scope_node->output_outer_is_local.push_back(true);
							new_scope_node->output_outer_indexes.push_back(final_it->second);
						}
					} else {
						if (include_state_distribution(generator) == 0) {
							potential_to_final_mapping[potential_it->second] = -1;
						} else {
							int new_state_index = new_scope->num_local_states;
							new_scope->num_local_states++;
							new_scope->original_local_state_ids.push_back(potential_innermost_state_ids[potential_it->second]);

							potential_to_final_mapping[potential_it->second] = new_state_index;

							new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
							new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
							new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
							if (potential_to_outer_mapping[potential_it->second].second.first) {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
							} else {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
							}
							new_potential_scope_node->input_outer_indexes.push_back(
								(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
							new_potential_scope_node->input_init_vals.push_back(0.0);
							new_potential_scope_node->input_init_index_vals.push_back(0.0);

							if (output_distribution(generator) == 0) {
								new_potential_scope_node->output_inner_indexes.push_back(new_state_index);
								new_potential_scope_node->output_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
								if (potential_to_outer_mapping[potential_it->second].second.first) {
									new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_LOCAL);
								} else {
									new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_INPUT);
								}
								new_potential_scope_node->output_outer_indexes.push_back(
									(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
							}

							new_scope_node->output_inner_is_local.push_back(original_scope_node->output_inner_is_local[o_index]);
							new_scope_node->output_inner_indexes.push_back(original_scope_node->output_inner_indexes[o_index]);
							new_scope_node->output_outer_is_local.push_back(true);
							new_scope_node->output_outer_indexes.push_back(new_state_index);
						}
					}
				}
			}

			new_scope_node->is_loop = original_scope_node->is_loop;
			new_scope_node->continue_score_mod = original_scope_node->continue_score_mod;
			new_scope_node->halt_score_mod = original_scope_node->halt_score_mod;
			new_scope_node->decision_standard_deviation = original_scope_node->decision_standard_deviation;
			new_scope_node->max_iters = original_scope_node->max_iters;

			for (int s_index = 0; s_index < (int)original_scope_node->loop_state_is_local.size(); s_index++) {
				map<pair<bool,int>, int>::iterator potential_it = potential_state_mappings[possible_scope_contexts[n_index].size()-1]
					.find({original_scope_node->loop_state_is_local[s_index], original_scope_node->loop_state_indexes[s_index]});
				if (potential_it != potential_state_mappings[possible_scope_contexts[n_index].size()-1].end()) {
					map<int, int>::iterator final_it = potential_to_final_mapping.find(potential_it->second);
					if (final_it != potential_to_final_mapping.end()) {
						if (final_it->second != -1) {
							new_scope_node->loop_state_is_local.push_back(true);
							new_scope_node->loop_state_indexes.push_back(final_it->second);
							new_scope_node->loop_continue_weights.push_back(original_scope_node->loop_continue_weights[s_index]);
							new_scope_node->loop_halt_weights.push_back(original_scope_node->loop_halt_weights[s_index]);
						}
					} else {
						if (include_state_distribution(generator) == 0) {
							potential_to_final_mapping[potential_it->second] = -1;
						} else {
							int new_state_index = new_scope->num_local_states;
							new_scope->num_local_states++;
							new_scope->original_local_state_ids.push_back(potential_innermost_state_ids[potential_it->second]);

							potential_to_final_mapping[potential_it->second] = new_state_index;

							new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
							new_potential_scope_node->input_inner_indexes.push_back(new_state_index);
							new_potential_scope_node->input_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
							if (potential_to_outer_mapping[potential_it->second].second.first) {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_LOCAL);
							} else {
								new_potential_scope_node->input_outer_types.push_back(OUTER_TYPE_INPUT);
							}
							new_potential_scope_node->input_outer_indexes.push_back(
								(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
							new_potential_scope_node->input_init_vals.push_back(0.0);
							new_potential_scope_node->input_init_index_vals.push_back(0.0);

							if (output_distribution(generator) == 0) {
								new_potential_scope_node->output_inner_indexes.push_back(new_state_index);
								new_potential_scope_node->output_scope_depths.push_back(potential_to_outer_mapping[potential_it->second].first);
								if (potential_to_outer_mapping[potential_it->second].second.first) {
									new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_LOCAL);
								} else {
									new_potential_scope_node->output_outer_types.push_back(OUTER_TYPE_INPUT);
								}
								new_potential_scope_node->output_outer_indexes.push_back(
									(void*)((long)potential_to_outer_mapping[potential_it->second].second.second));
							}

							new_scope_node->loop_state_is_local.push_back(true);
							new_scope_node->loop_state_indexes.push_back(new_state_index);
							new_scope_node->loop_continue_weights.push_back(original_scope_node->loop_continue_weights[s_index]);
							new_scope_node->loop_halt_weights.push_back(original_scope_node->loop_halt_weights[s_index]);
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

	new_scope->starting_node_id = 0;
	new_scope->starting_node = new_nodes[0];

	return new_potential_scope_node;
}
