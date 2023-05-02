#include "loop_fold_to_scope.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void loop_fold_to_scope(LoopFold* loop_fold,
						int& new_scope_id) {
	int num_inner_networks = loop_fold->sum_inner_inputs
		+ loop_fold->curr_num_new_inner_states
		+ loop_fold->num_states;
	int total_num_states = loop_fold->sum_inner_inputs
		+ loop_fold->curr_num_new_inner_states
		+ loop_fold->num_states
		+ loop_fold->curr_num_new_outer_states;

	int new_num_states = 0;
	vector<bool> new_is_initialized_locally;
	vector<int> state_index_mapping(total_num_states, -1);
	for (int s_index = 0; s_index < total_num_states; s_index++) {
		if (s_index >= loop_fold->sum_inner_inputs
				|| loop_fold->curr_inner_inputs_needed[s_index]) {
			state_index_mapping[s_index] = new_num_states;
			new_num_states++;

			if (s_index < loop_fold->sum_inner_inputs + loop_fold->curr_num_new_inner_states) {
				new_is_initialized_locally.push_back(true);
			} else {
				new_is_initialized_locally.push_back(false);
			}
		}
	}

	vector<AbstractNode*> new_nodes;
	for (int f_index = 0; f_index < loop_fold->sequence_length; f_index++) {
		if (loop_fold->is_inner_scope[f_index]) {
			vector<int> pre_state_network_target_indexes;
			vector<StateNetwork*> pre_state_networks;
			for (int s_index = 0; s_index < loop_fold->inner_input_start_indexes[f_index] + loop_fold->num_inner_inputs[f_index]; s_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][s_index]) {
					pre_state_network_target_indexes.push_back(state_index_mapping[s_index]);

					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][s_index];
					state_network->new_sequence_finalize();
					// remove back to front
					for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
						if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
							state_network->remove_state(ss_index);
						}
					}
					pre_state_networks.push_back(state_network);
				}
			}

			int inner_scope_id = loop_fold->existing_scope_ids[f_index];

			vector<int> inner_input_indexes;
			vector<int> inner_input_target_indexes;
			for (int i_index = 0; i_index < loop_fold->num_inner_inputs[f_index]; i_index++) {
				if (loop_fold->curr_inner_inputs_needed[loop_fold->inner_input_start_indexes[f_index]+i_index]) {
					inner_input_indexes.push_back(state_index_mapping[loop_fold->inner_input_start_indexes[f_index]+i_index]);
					inner_input_target_indexes.push_back(i_index);
				}
			}

			// inner scopes already updated with new inner
			if (loop_fold->curr_inner_scopes_needed.find(inner_scope_id) != loop_fold->curr_inner_scopes_needed.end()) {
				Scope* inner_scope = solution->scopes[inner_scope_id];
				for (int i_index = 0; i_index < loop_fold->curr_num_new_inner_states; i_index++) {
					inner_input_indexes.push_back(state_index_mapping[loop_fold->sum_inner_inputs+i_index]);
					inner_input_target_indexes.push_back(inner_scope->num_states-loop_fold->curr_num_new_inner_states+i_index);
				}
			}

			vector<int> post_state_network_target_indexes;
			vector<StateNetwork*> post_state_networks;
			for (int s_index = loop_fold->inner_input_start_indexes[f_index] + loop_fold->num_inner_inputs[f_index]; s_index < num_inner_networks; s_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][s_index]) {
					post_state_network_target_indexes.push_back(state_index_mapping[s_index]);

					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][s_index];
					state_network->new_sequence_finalize();
					// remove back to front
					for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
						if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
							state_network->remove_state(ss_index);
						}
					}
					post_state_networks.push_back(state_network);
				}
			}

			StateNetwork* score_network = loop_fold->curr_score_networks[f_index];
			score_network->new_sequence_finalize();
			for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
				if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
					score_network->remove_state(ss_index);
				}
			}

			ScopeNode* node = new ScopeNode(pre_state_network_target_indexes,
											pre_state_networks,
											inner_scope_id,
											inner_input_indexes,
											inner_input_target_indexes,
											loop_fold->inner_scope_scale_mods[f_index],
											post_state_network_target_indexes,
											post_state_networks,
											score_network);
			new_nodes.push_back(node);
		} else {
			vector<int> state_network_target_indexes;
			vector<StateNetwork*> state_networks;
			for (int s_index = 0; s_index < num_inner_networks; s_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][s_index]) {
					state_network_target_indexes.push_back(state_index_mapping[s_index]);

					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][s_index];
					state_network->new_sequence_finalize();
					for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
						if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
							state_network->remove_state(ss_index);
						}
					}
					state_networks.push_back(state_network);
				}
			}

			StateNetwork* score_network = loop_fold->curr_score_networks[f_index];
			score_network->new_sequence_finalize();
			for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
				if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
					score_network->remove_state(ss_index);
				}
			}

			ActionNode* node = new ActionNode(state_network_target_indexes,
											  state_networks,
											  score_network);
			new_nodes.push_back(node);
		}
	}

	vector<StateNetwork*> starting_state_networks;
	for (int i_index = 0; i_index < loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states; i_index++) {
		if (i_index >= loop_fold->sum_inner_inputs
				|| loop_fold->curr_inner_inputs_needed[i_index]) {
			StateNetwork* state_network = loop_fold->curr_starting_state_networks[i_index];
			state_network->new_sequence_finalize();
			for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
				if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
					state_network->remove_state(ss_index);
				}
			}
			starting_state_networks.push_back(state_network);
		}
	}

	StateNetwork* continue_score_network = loop_fold->curr_continue_score_network;
	continue_score_network->new_sequence_finalize();
	for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
		if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
			continue_score_network->remove_state(ss_index);
		}
	}

	StateNetwork* continue_misguess_network = loop_fold->curr_continue_misguess_network;
	continue_misguess_network->new_sequence_finalize();
	for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
		if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
			continue_misguess_network->remove_state(ss_index);
		}
	}

	StateNetwork* halt_score_network = loop_fold->curr_halt_score_network;
	halt_score_network->new_sequence_finalize();
	for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
		if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
			halt_score_network->remove_state(ss_index);
		}
	}

	StateNetwork* halt_misguess_network = loop_fold->curr_halt_misguess_network;
	halt_misguess_network->new_sequence_finalize();
	for (int ss_index = loop_fold->sum_inner_inputs-1; ss_index >= 0; ss_index--) {
		if (!loop_fold->curr_inner_inputs_needed[ss_index]) {
			halt_misguess_network->remove_state(ss_index);
		}
	}

	Scope* new_scope = new Scope(new_num_states,
								 new_is_initialized_locally,
								 true,
								 starting_state_networks,
								 continue_score_network,
								 continue_misguess_network,
								 halt_score_network,
								 halt_misguess_network,
								 loop_fold->curr_average_score,
								 loop_fold->curr_score_variance,
								 loop_fold->curr_average_misguess,
								 loop_fold->curr_misguess_variance,
								 new_nodes);
	solution->scopes.push_back(new_scope);
	new_scope_id = (int)solution->scopes.size()-1;
}
