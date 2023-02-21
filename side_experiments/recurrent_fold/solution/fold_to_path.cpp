#include "fold_to_path.h"

#include <iostream>

#include "scope.h"

using namespace std;

ScopePath* fold_to_path(Fold* fold) {
	vector<int> scope_inclusive_starts;
	vector<int> scope_inclusive_ends;
	vector<vector<bool>> is_inner_scope;
	vector<vector<int>> inner_scope_indexes;
	vector<int> scope_parents;
	vector<int> scope_top_layers;

	vector<int> starting_indexes(fold->sequence_length, -1);
	vector<int> scope_indexes(fold->sequence_length, -1);

	for (int s_index = 0; s_index < fold->curr_num_states; s_index++) {
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
						scope_top_layers.push_back(s_index);

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
				scope_top_layers.push_back(s_index);

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

	vector<Scope*> scopes;
	vector<vector<int>> input_index_reverse_mappings;
	for (int n_index = 0; n_index < (int)scope_inclusive_starts.size(); n_index++) {
		vector<bool> used_states(fold->curr_num_states, false);
		for (int f_index = scope_inclusive_starts[n_index]; f_index <= scope_inclusive_ends[n_index]; f_index++) {
			for (int s_index = 0; s_index < fold->curr_num_states; s_index++) {
				if (!fold->curr_state_not_needed_locally[f_index][s_index]) {
					used_states[s_index] = true;
				}
			}
		}

		int parent_layer;
		if (scope_parents[n_index] == -1) {
			parent_layer = -1;
		} else {
			parent_layer = scope_top_layers[scope_parents[n_index]];
		}

		int new_num_input_states = 0;
		int new_num_local_states = 0;
		vector<bool> state_is_local_mapping(fold->curr_num_states);
		vector<int> state_index_mapping(fold->curr_num_states, -1);
		vector<int> input_index_reverse_mapping;
		for (int s_index = 0; s_index < fold->curr_num_states; s_index++) {
			if (used_states[s_index]) {
				if (s_index > parent_layer) {
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

		int new_sequence_length = (int)is_inner_scope[n_index].size();
		vector<Scope*> new_scopes;
		vector<vector<bool>> new_inner_input_is_local;
		vector<vector<int>> new_inner_input_indexes;
		vector<vector<bool>> new_state_network_target_is_local;
		vector<vector<int>> new_state_network_target_indexes;
		vector<vector<Network*>> new_state_networks;
		vector<bool> new_has_score_network;
		vector<Network*> new_score_networks;
		int f_index = scope_inclusive_starts[n_index];
		for (int a_index = 0; a_index < new_sequence_length; a_index++) {
			if (is_inner_scope[n_index][a_index]) {
				// inner scope already initialized
				new_scopes.push_back(scopes[inner_scope_indexes[n_index][a_index]]);
				new_inner_input_is_local.push_back(vector<bool>());
				new_inner_input_indexes.push_back(vector<int>());
				for (int i_index = 0; i_index < scopes[inner_scope_indexes[n_index][a_index]]->num_input_states; i_index++) {
					int original_index = input_index_reverse_mappings[inner_scope_indexes[n_index][a_index]][i_index];
					if (original_index > parent_layer) {
						new_inner_input_is_local[a_index].push_back(true);
					} else {
						new_inner_input_is_local[a_index].push_back(false);
					}
					new_inner_input_indexes[a_index].push_back(state_index_mapping[original_index]);
				}

				// empty for is_inner_scope
				new_state_network_target_is_local.push_back(vector<bool>());
				new_state_network_target_indexes.push_back(vector<int>());
				new_state_networks.push_back(vector<Network*>());

				// don't add additional score network as is not existing
				new_has_score_network.push_back(false);
				new_score_networks.push_back(NULL);

				f_index = scope_inclusive_ends[inner_scope_indexes[n_index][a_index]];
			} else {
				new_state_network_target_is_local.push_back(vector<bool>());
				new_state_network_target_indexes.push_back(vector<int>());
				new_state_networks.push_back(vector<Network*>());
				for (int s_index = 0; s_index < fold->curr_num_states; s_index++) {
					if (!fold->curr_state_networks_not_needed[f_index][s_index]) {
						if (s_index > parent_layer) {
							new_state_network_target_is_local[a_index].push_back(true);
						} else {
							new_state_network_target_is_local[a_index].push_back(false);
						}
						new_state_network_target_indexes[a_index].push_back(state_index_mapping[s_index]);

						new_state_networks[a_index].push_back(fold->curr_state_networks[f_index][s_index]);
						fold->curr_state_networks[f_index][s_index] = NULL;
						for (int ss_index = s_index; ss_index >= 0; ss_index--) {
							if (!used_states[ss_index]) {
								// TODO: need to offset by observation size
								new_state_networks[a_index].back()->remove_input(1+ss_index);
							}
						}
					}
				}

				new_has_score_network.push_back(true);
				new_score_networks.push_back(fold->curr_score_networks[f_index]);
				fold->curr_score_networks[f_index] = NULL;
				for (int s_index = fold->curr_num_states-1; s_index >= 0; s_index--) {
					if (!used_states[s_index]) {
						new_score_networks[a_index]->remove_input(s_index);
					}
				}

				new_scopes.push_back(NULL);
				new_inner_input_is_local.push_back(vector<bool>());
				new_inner_input_indexes.push_back(vector<int>());
			}

			f_index++;
		}

		ScopePath* new_scope_path = new ScopePath(new_num_input_states,
												  new_num_local_states,
												  new_sequence_length,
												  is_inner_scope[n_index],
												  new_scopes,
												  new_inner_input_is_local,
												  new_inner_input_indexes,
												  new_state_network_target_is_local,
												  new_state_network_target_indexes,
												  new_state_networks,
												  new_has_score_network,
												  new_score_networks);

		Scope* new_scope = new Scope(new_num_input_states,
									 0,
									 false,
									 NULL,
									 NULL,
									 vector<Network*>{NULL},
									 vector<bool>{false},
									 vector<ScopePath*>{new_scope_path},
									 vector<Fold*>{NULL},
									 vector<int>{100000});

		scopes.push_back(new_scope);
		input_index_reverse_mappings.push_back(input_index_reverse_mapping);
	}

	if (scope_inclusive_starts.back() == 0 && scope_inclusive_ends.back() == fold->sequence_length-1) {
		ScopePath* result = scopes.back()->branches[0];
		scopes.back()->branches[0] = NULL;
		delete scopes.back();
		return result;
	} else {
		vector<bool> outer_is_inner_scope;
		vector<Scope*> outer_scopes;
		vector<vector<bool>> outer_inner_input_is_local;
		vector<vector<int>> outer_inner_input_indexes;
		vector<vector<bool>> outer_state_network_target_is_local;
		vector<vector<int>> outer_state_network_target_indexes;
		vector<vector<Network*>> outer_state_networks;
		vector<bool> outer_has_score_network;
		vector<Network*> outer_score_networks;

		int inner_scope_index = -1;
		for (int f_index = 0; f_index < fold->sequence_length; f_index++) {
			if (scope_indexes[f_index] == -1) {
				outer_is_inner_scope.push_back(false);
				outer_scopes.push_back(NULL);

				outer_inner_input_is_local.push_back(vector<bool>());
				outer_inner_input_indexes.push_back(vector<int>());
				outer_state_network_target_is_local.push_back(vector<bool>());
				outer_state_network_target_indexes.push_back(vector<int>());
				outer_state_networks.push_back(vector<Network*>());

				// don't add score network as currently has no effect
				outer_has_score_network.push_back(false);
				outer_score_networks.push_back(NULL);
			} else {
				if (inner_scope_index == scope_indexes[f_index]) {
					// do nothing
				} else {
					outer_is_inner_scope.push_back(true);
					outer_scopes.push_back(scopes[scope_indexes[f_index]]);

					outer_inner_input_is_local.push_back(vector<bool>());
					outer_inner_input_indexes.push_back(vector<int>());
					outer_state_network_target_is_local.push_back(vector<bool>());
					outer_state_network_target_indexes.push_back(vector<int>());
					outer_state_networks.push_back(vector<Network*>());

					outer_has_score_network.push_back(false);
					outer_score_networks.push_back(NULL);
				}
			}
			inner_scope_index = scope_indexes[f_index];
		}

		ScopePath* outer_scope_path = new ScopePath(0,
													0,
													(int)outer_is_inner_scope.size(),
													outer_is_inner_scope,
													outer_scopes,
													outer_inner_input_is_local,
													outer_inner_input_indexes,
													outer_state_network_target_is_local,
													outer_state_network_target_indexes,
													outer_state_networks,
													outer_has_score_network,
													outer_score_networks);

		return outer_scope_path;
	}
}
