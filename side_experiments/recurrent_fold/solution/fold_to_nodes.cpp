#include "fold_to_nodes.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void fold_to_nodes(Scope* parent_scope,
				   Fold* fold,
				   vector<AbstractNode*>& new_outer_nodes) {
	vector<int> scope_inclusive_starts;
	vector<int> scope_inclusive_ends;
	vector<vector<bool>> is_inner_scope;
	vector<vector<int>> inner_scope_indexes;
	vector<int> scope_parents;
	vector<int> scope_lowest_layers;

	vector<int> starting_indexes(fold->sequence_length, -1);
	vector<int> scope_indexes(fold->sequence_length, -1);

	for (int s_index = 0; s_index < fold->sum_inner_inputs+fold->curr_num_new_inner_states; s_index++) {
		int curr_start = -1;
		int inner_scope_index = -1;
		vector<bool> new_is_inner_scope;
		vector<int> new_inner_scope_indexes;
		for (int f_index = 0; f_index < fold->sequence_length; f_index++) {
			if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
				if (curr_start == -1) {
					if (starting_indexes[f_index] != -1) {
						curr_start = starting_indexes[f_index];
					} else {
						curr_start = f_index;
					}
				}
			}

			if (curr_start != -1) {
				if (inner_scope_index == -1) {
					if (scope_indexes[f_index] == -1) {
						new_is_inner_scope.push_back(false);
						new_inner_scope_indexes.push_back(-1);
					} else {
						new_is_inner_scope.push_back(true);
						new_inner_scope_indexes.push_back(scope_indexes[f_index]);
					}
				} else {
					if (scope_indexes[f_index] == -1) {
						new_is_inner_scope.push_back(false);
						new_inner_scope_indexes.push_back(-1);
					} else {
						if (inner_scope_index == scope_indexes[f_index]) {
							// do nothing
						} else {
							new_is_inner_scope.push_back(true);
							new_inner_scope_indexes.push_back(scope_indexes[f_index]);
						}
					}
				}
				inner_scope_index = scope_indexes[f_index];

				if (fold->curr_num_states_cleared[f_index] > s_index) {
					if (starting_indexes[f_index] == curr_start) {
						// matches existing scope
						// do nothing
					} else {
						scope_inclusive_starts.push_back(curr_start);
						scope_inclusive_ends.push_back(f_index);
						is_inner_scope.push_back(new_is_inner_scope);
						inner_scope_indexes.push_back(new_inner_scope_indexes);
						scope_parents.push_back(-1);
						scope_lowest_layers.push_back(s_index);

						for (int ff_index = curr_start; ff_index <= f_index; ff_index++) {
							starting_indexes[ff_index] = curr_start;
							scope_indexes[ff_index] = (int)scope_inclusive_starts.size()-1;
						}
						for (int i_index = 0; i_index < (int)new_inner_scope_indexes.size(); i_index++) {
							if (new_inner_scope_indexes[i_index] != -1) {
								scope_parents[new_inner_scope_indexes[i_index]] = (int)scope_inclusive_starts.size()-1;
							}
						}
					}

					curr_start = -1;
					inner_scope_index = -1;
					new_is_inner_scope.clear();
					new_inner_scope_indexes.clear();
				}
			}
		}
		if (curr_start != -1) {
			if (starting_indexes[fold->sequence_length-1] == curr_start) {
				// matches existing scope
				// do nothing
			} else {
				scope_inclusive_starts.push_back(curr_start);
				scope_inclusive_ends.push_back(fold->sequence_length-1);
				is_inner_scope.push_back(new_is_inner_scope);
				inner_scope_indexes.push_back(new_inner_scope_indexes);
				scope_parents.push_back(-1);
				scope_lowest_layers.push_back(s_index);

				for (int ff_index = curr_start; ff_index <= fold->sequence_length-1; ff_index++) {
					starting_indexes[ff_index] = curr_start;
					scope_indexes[ff_index] = (int)scope_inclusive_starts.size()-1;
				}
				for (int i_index = 0; i_index < (int)new_inner_scope_indexes.size(); i_index++) {
					if (new_inner_scope_indexes[i_index] != -1) {
						scope_parents[new_inner_scope_indexes[i_index]] = (int)scope_inclusive_starts.size()-1;
					}
				}
			}
		}
	}

	int total_num_states = fold->sum_inner_inputs
		+ fold->curr_num_new_inner_states
		+ fold->num_sequence_local_states
		+ fold->num_sequence_input_states;

	vector<int> new_scope_ids;
	vector<vector<int>> input_index_reverse_mappings;
	for (int n_index = 0; n_index < (int)scope_inclusive_starts.size(); n_index++) {
		vector<bool> used_states(total_num_states, false);
		for (int f_index = scope_inclusive_starts[n_index]; f_index <= scope_inclusive_ends[n_index]; f_index++) {
			for (int s_index = 0; s_index < total_num_states; s_index++) {
				if (!fold->curr_state_not_needed_locally[f_index][s_index]) {
					used_states[s_index] = true;
				}
			}
		}

		int parent_lowest_layer;
		if (scope_parents[n_index] == -1) {
			parent_lowest_layer = fold->sum_inner_inputs+fold->curr_num_new_inner_states;
		} else {
			parent_lowest_layer = scope_lowest_layers[scope_parents[n_index]];
		}

		int new_num_local_states = 0;
		int new_num_input_states = 0;
		vector<bool> state_is_local_mapping(total_num_states);
		vector<int> state_index_mapping(total_num_states, -1);
		vector<int> input_index_reverse_mapping;
		for (int s_index = 0; s_index < total_num_states; s_index++) {
			if (used_states[s_index]) {
				if (s_index < parent_lowest_layer) {
					state_is_local_mapping[s_index] = true;
					state_index_mapping[s_index] = new_num_local_states;
					new_num_local_states++;
				} else {
					state_is_local_mapping[s_index] = false;
					state_index_mapping[s_index] = new_num_input_states;
					new_num_input_states++;
					input_index_reverse_mapping.push_back(s_index);
				}
			}
		}

		vector<AbstractNode*> new_nodes;
		int f_index = scope_inclusive_starts[n_index];
		for (int a_index = 0; a_index < (int)is_inner_scope[n_index].size(); a_index++) {
			if (is_inner_scope[n_index][a_index]) {
				// inner scope already initialized

				// pre_state_networks/post_state_networks empty

				int inner_scope_id = new_scope_ids[inner_scope_indexes[n_index][a_index]];

				vector<bool> inner_input_is_local;
				vector<int> inner_input_indexes;
				vector<int> inner_input_target_indexes;
				for (int i_index = 0; i_index < solution->scopes[inner_scope_id]->num_input_states; i_index++) {
					int original_index = input_index_reverse_mappings[inner_scope_indexes[n_index][a_index]][i_index];
					if (original_index < parent_lowest_layer) {
						inner_input_is_local.push_back(true);
					} else {
						inner_input_is_local.push_back(false);
					}
					inner_input_indexes.push_back(state_index_mapping[original_index]);
					inner_input_target_indexes.push_back(i_index);
				}

				StateNetwork* score_network = new StateNetwork(0,
															   new_num_local_states,
															   new_num_input_states,
															   0,
															   0,
															   20);

				ScopeNode* node = new ScopeNode(vector<bool>(),
												vector<int>(),
												vector<StateNetwork*>(),
												inner_scope_id,
												inner_input_is_local,
												inner_input_indexes,
												inner_input_target_indexes,
												vector<bool>(),
												vector<int>(),
												vector<StateNetwork*>(),
												score_network);
				new_nodes.push_back(node);

				f_index = scope_inclusive_ends[inner_scope_indexes[n_index][a_index]];
			} else {
				if (fold->is_inner_scope[f_index]) {
					vector<bool> pre_state_network_target_is_local;
					vector<int> pre_state_network_target_indexes;
					vector<StateNetwork*> pre_state_networks;
					for (int s_index = 0; s_index < fold->inner_input_start_indexes[f_index] + fold->num_inner_inputs[f_index]; s_index++) {
						if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
							if (s_index < parent_lowest_layer) {
								pre_state_network_target_is_local.push_back(true);
							} else {
								pre_state_network_target_is_local.push_back(false);
							}
							pre_state_network_target_indexes.push_back(state_index_mapping[s_index]);

							StateNetwork* state_network = fold->curr_state_networks[f_index][s_index];
							state_network->split_new_inner(parent_lowest_layer);
							// remove back to front
							for (int ss_index = total_num_states-1; ss_index >= 0; ss_index--) {
								if (!used_states[ss_index]) {
									if (ss_index < parent_lowest_layer) {
										state_network->remove_local(ss_index);
									} else {
										state_network->remove_input(ss_index-parent_lowest_layer);
									}
								}
							}
							pre_state_networks.push_back(state_network);
						}
					}

					int inner_scope_id = fold->existing_scope_ids[f_index];

					vector<bool> inner_input_is_local;
					vector<int> inner_input_indexes;
					vector<int> inner_input_target_indexes;
					for (int s_index = fold->inner_input_start_indexes[f_index];
							s_index < fold->inner_input_start_indexes[f_index] + fold->num_inner_inputs[f_index]; s_index++) {
						if (s_index < parent_lowest_layer) {
							inner_input_is_local.push_back(true);
						} else {
							inner_input_is_local.push_back(false);
						}
						inner_input_indexes.push_back(state_index_mapping[s_index]);
						inner_input_target_indexes.push_back((int)inner_input_target_indexes.size());	// 0 to num_inner_inputs
					}

					vector<bool> post_state_network_target_is_local;
					vector<int> post_state_network_target_indexes;
					vector<StateNetwork*> post_state_networks;
					for (int s_index = fold->inner_input_start_indexes[f_index] + fold->num_inner_inputs[f_index]; s_index < total_num_states; s_index++) {
						if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
							if (s_index < parent_lowest_layer) {
								post_state_network_target_is_local.push_back(true);
							} else {
								post_state_network_target_is_local.push_back(false);
							}
							post_state_network_target_indexes.push_back(state_index_mapping[s_index]);

							StateNetwork* state_network = fold->curr_state_networks[f_index][s_index];
							state_network->split_new_inner(parent_lowest_layer);
							// remove back to front
							for (int ss_index = total_num_states-1; ss_index >= 0; ss_index--) {
								if (!used_states[ss_index]) {
									if (ss_index < parent_lowest_layer) {
										state_network->remove_local(ss_index);
									} else {
										state_network->remove_input(ss_index-parent_lowest_layer);
									}
								}
							}
							post_state_networks.push_back(state_network);
						}
					}

					StateNetwork* score_network = fold->curr_score_networks[f_index];
					score_network->split_new_inner(parent_lowest_layer);
					for (int ss_index = total_num_states-1; ss_index >= 0; ss_index--) {
						if (!used_states[ss_index]) {
							if (ss_index < parent_lowest_layer) {
								score_network->remove_local(ss_index);
							} else {
								score_network->remove_input(ss_index-parent_lowest_layer);
							}
						}
					}

					ScopeNode* node = new ScopeNode(pre_state_network_target_is_local,
													pre_state_network_target_indexes,
													pre_state_networks,
													inner_scope_id,
													inner_input_is_local,
													inner_input_indexes,
													inner_input_target_indexes,
													post_state_network_target_is_local,
													post_state_network_target_indexes,
													post_state_networks,
													score_network);
					new_nodes.push_back(node);
				} else {
					vector<bool> state_network_target_is_local;
					vector<int> state_network_target_indexes;
					vector<StateNetwork*> state_networks;
					for (int s_index = 0; s_index < total_num_states; s_index++) {
						if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
							if (s_index < parent_lowest_layer) {
								state_network_target_is_local.push_back(true);
							} else {
								state_network_target_is_local.push_back(false);
							}
							state_network_target_indexes.push_back(state_index_mapping[s_index]);

							StateNetwork* state_network = fold->curr_state_networks[f_index][s_index];
							state_network->split_new_inner(parent_lowest_layer);
							// remove back to front
							for (int ss_index = total_num_states-1; ss_index >= 0; ss_index--) {
								if (!used_states[ss_index]) {
									if (ss_index < parent_lowest_layer) {
										state_network->remove_local(ss_index);
									} else {
										state_network->remove_input(ss_index-parent_lowest_layer);
									}
								}
							}
							state_networks.push_back(state_network);
						}
					}

					StateNetwork* score_network = fold->curr_score_networks[f_index];
					score_network->split_new_inner(parent_lowest_layer);
					for (int ss_index = total_num_states-1; ss_index >= 0; ss_index--) {
						if (!used_states[ss_index]) {
							if (ss_index < parent_lowest_layer) {
								score_network->remove_local(ss_index);
							} else {
								score_network->remove_input(ss_index-parent_lowest_layer);
							}
						}
					}

					ActionNode* node = new ActionNode(state_network_target_is_local,
													  state_network_target_indexes,
													  state_networks,
													  score_network);
					new_nodes.push_back(node);
				}
			}

			f_index++;
		}

		Scope* new_scope = new Scope(new_num_local_states,
									 new_num_input_states,
									 false,
									 NULL,
									 NULL,
									 new_nodes);
		solution->scopes.push_back(new_scope);
		int new_scope_id = (int)solution->scopes.size()-1;
		new_scope->id = new_scope_id;

		new_scope_ids.push_back(new_scope_id);
		input_index_reverse_mappings.push_back(input_index_reverse_mapping);
	}

	int inner_scope_index = -1;
	for (int f_index = 0; f_index < fold->sequence_length; f_index++) {
		if (scope_indexes[f_index] == -1) {
			// fold->is_inner_scope[f_index] == false

			vector<bool> state_network_target_is_local;
			vector<int> state_network_target_indexes;
			vector<StateNetwork*> state_networks;
			for (int s_index = 0; s_index < total_num_states; s_index++) {
				if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
					int target_index = s_index - (fold->sum_inner_inputs + fold->curr_num_new_inner_states);
					if (target_index < fold->num_sequence_local_states) {
						state_network_target_is_local.push_back(true);
						state_network_target_indexes.push_back(target_index);
					} else {
						target_index -= fold->num_sequence_local_states;
						if (target_index < fold->num_sequence_input_states) {
							state_network_target_is_local.push_back(false);
							state_network_target_indexes.push_back(target_index);
						} else {
							target_index -= fold->num_sequence_input_states;
							if (fold->scope_context.size() == 1) {
								state_network_target_is_local.push_back(true);
								state_network_target_indexes.push_back(parent_scope->num_local_states + target_index);
							} else {
								state_network_target_is_local.push_back(false);
								state_network_target_indexes.push_back(parent_scope->num_input_states + target_index);
							}
						}
					}

					StateNetwork* state_network = fold->curr_state_networks[f_index][s_index];
					state_network->update_state_sizes(parent_scope->num_local_states,
													  parent_scope->num_input_states);
					if (fold->scope_context.size() == 1) {
						state_network->new_outer_to_local();
					} else {
						state_network->new_outer_to_input();
					}
					state_networks.push_back(state_network);
				}
			}

			StateNetwork* score_network = fold->curr_score_networks[f_index];
			score_network->update_state_sizes(parent_scope->num_local_states,
											  parent_scope->num_input_states);
			if (fold->scope_context.size() == 1) {
				score_network->new_outer_to_local();
			} else {
				score_network->new_outer_to_input();
			}

			ActionNode* node = new ActionNode(state_network_target_is_local,
											  state_network_target_indexes,
											  state_networks,
											  score_network);
			new_outer_nodes.push_back(node);
		} else {
			if (inner_scope_index == scope_indexes[f_index]) {
				// do nothing
			} else {
				// inner scope already initialized

				// pre_state_networks/post_state_networks empty

				int inner_scope_id = new_scope_ids[scope_indexes[f_index]];

				vector<bool> inner_input_is_local;
				vector<int> inner_input_indexes;
				vector<int> inner_input_target_indexes;
				for (int i_index = 0; i_index < solution->scopes[inner_scope_id]->num_input_states; i_index++) {
					int original_index = input_index_reverse_mappings[scope_indexes[f_index]][i_index];
					original_index -= (fold->sum_inner_inputs + fold->curr_num_new_inner_states);
					if (original_index < fold->num_sequence_local_states) {
						inner_input_is_local.push_back(true);
						inner_input_indexes.push_back(original_index);
					} else {
						original_index -= fold->num_sequence_local_states;
						if (original_index < fold->num_sequence_input_states) {
							inner_input_is_local.push_back(false);
							inner_input_indexes.push_back(original_index);
						} else {
							original_index -= fold->num_sequence_input_states;
							if (fold->scope_context.size() == 1) {
								inner_input_is_local.push_back(true);
								// new_outer not yet added to parent_scope
								inner_input_indexes.push_back(parent_scope->num_local_states + original_index);
							} else {
								inner_input_is_local.push_back(false);
								inner_input_indexes.push_back(parent_scope->num_input_states + original_index);
							}
						}
					}
					inner_input_target_indexes.push_back(i_index);
				}

				StateNetwork* score_network;
				if (fold->scope_context.size() == 1) {
					score_network = new StateNetwork(0,
													 parent_scope->num_local_states + fold->curr_num_new_outer_states,
													 parent_scope->num_input_states,
													 0,
													 0,
													 20);
				} else {
					score_network = new StateNetwork(0,
													 parent_scope->num_local_states,
													 parent_scope->num_input_states + fold->curr_num_new_outer_states,
													 0,
													 0,
													 20);
				}

				ScopeNode* node = new ScopeNode(vector<bool>(),
												vector<int>(),
												vector<StateNetwork*>(),
												inner_scope_id,
												inner_input_is_local,
												inner_input_indexes,
												inner_input_target_indexes,
												vector<bool>(),
												vector<int>(),
												vector<StateNetwork*>(),
												score_network);
				new_outer_nodes.push_back(node);
			}
		}
		inner_scope_index = scope_indexes[f_index];
	}
}
