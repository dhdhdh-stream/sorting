#include "loop_fold_to_scope.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "scope_node.h"

using namespace std;

void loop_fold_to_scope(LoopFold* loop_fold,
						int& new_scope_id) {
	vector<AbstractNode*> new_nodes;
	for (int f_index = 0; f_index < loop_fold->sequence_length; f_index++) {
		if (loop_fold->is_inner_scope[f_index]) {
			vector<bool> pre_state_network_target_is_local;
			vector<int> pre_state_network_target_indexes;
			vector<StateNetwork*> pre_state_networks;
			for (int s_index = 0; s_index < loop_fold->inner_input_start_indexes[f_index] + loop_fold->num_inner_inputs[f_index]; s_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][s_index]) {
					pre_state_network_target_is_local.push_back(true);
					pre_state_network_target_indexes.push_back(s_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][s_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					pre_state_networks.push_back(state_network);
				}
			}

			int inner_scope_id = loop_fold->existing_scope_ids[f_index];

			vector<bool> inner_input_is_local;
			vector<int> inner_input_indexes;
			vector<int> inner_input_target_indexes;
			for (int s_index = loop_fold->inner_input_start_indexes[f_index];
					s_index < loop_fold->inner_input_start_indexes[f_index] + loop_fold->num_inner_inputs[f_index]; s_index++) {
				inner_input_is_local.push_back(true);
				inner_input_indexes.push_back(s_index);
				inner_input_target_indexes.push_back((int)inner_input_target_indexes.size());	// 0 to num_inner_inputs
			}

			// inner scopes already updated
			if (loop_fold->curr_inner_scopes_needed.find(inner_scope_id) != loop_fold->curr_inner_scopes_needed.end()) {
				Scope* inner_scope = solution->scopes[inner_scope_id];
				for (int i_index = 0; i_index < loop_fold->curr_num_new_inner_states; i_index++) {
					inner_input_is_local.push_back(true);
					inner_input_indexes.push_back(loop_fold->sum_inner_inputs+i_index);
					inner_input_target_indexes.push_back(inner_scope->num_input_states-loop_fold->curr_num_new_inner_states+i_index);
				}
			}

			vector<bool> post_state_network_target_is_local;
			vector<int> post_state_network_target_indexes;
			vector<StateNetwork*> post_state_networks;
			for (int i_index = loop_fold->inner_input_start_indexes[f_index] + loop_fold->num_inner_inputs[f_index];
					i_index < loop_fold->sum_inner_inputs + loop_fold->curr_num_new_inner_states; i_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][i_index]) {
					post_state_network_target_is_local.push_back(true);
					post_state_network_target_indexes.push_back(i_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][i_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					post_state_networks.push_back(state_network);
				}
			}
			for (int l_index = 0; l_index < loop_fold->num_local_states; l_index++) {
				int state_index = loop_fold->sum_inner_inputs
					+ loop_fold->curr_num_new_inner_states
					+ l_index;
				if (!loop_fold->curr_state_networks_not_needed[f_index][state_index]) {
					post_state_network_target_is_local.push_back(false);
					post_state_network_target_indexes.push_back(l_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][state_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					post_state_networks.push_back(state_network);
				}
			}
			for (int i_index = 0; i_index < loop_fold->num_input_states; i_index++) {
				int state_index = loop_fold->sum_inner_inputs
					+ loop_fold->curr_num_new_inner_states
					+ loop_fold->num_local_states
					+ i_index;
				if (!loop_fold->curr_state_networks_not_needed[f_index][state_index]) {
					post_state_network_target_is_local.push_back(false);
					post_state_network_target_indexes.push_back(loop_fold->num_local_states+i_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][state_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					post_state_networks.push_back(state_network);
				}
			}

			StateNetwork* score_network = loop_fold->curr_score_networks[f_index];
			score_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

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
			for (int i_index = 0; i_index < loop_fold->sum_inner_inputs + loop_fold->curr_num_new_inner_states; i_index++) {
				if (!loop_fold->curr_state_networks_not_needed[f_index][i_index]) {
					state_network_target_is_local.push_back(true);
					state_network_target_indexes.push_back(i_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][i_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					state_networks.push_back(state_network);
				}
			}
			for (int l_index = 0; l_index < loop_fold->num_local_states; l_index++) {
				int state_index = loop_fold->sum_inner_inputs
					+ loop_fold->curr_num_new_inner_states
					+ l_index;
				if (!loop_fold->curr_state_networks_not_needed[f_index][state_index]) {
					state_network_target_is_local.push_back(false);
					state_network_target_indexes.push_back(l_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][state_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					state_networks.push_back(state_network);
				}
			}
			for (int i_index = 0; i_index < loop_fold->num_input_states; i_index++) {
				int state_index = loop_fold->sum_inner_inputs
					+ loop_fold->curr_num_new_inner_states
					+ loop_fold->num_local_states
					+ i_index;
				if (!loop_fold->curr_state_networks_not_needed[f_index][state_index]) {
					state_network_target_is_local.push_back(false);
					state_network_target_indexes.push_back(loop_fold->num_local_states+i_index);
					StateNetwork* state_network = loop_fold->curr_state_networks[f_index][state_index];
					state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
					state_networks.push_back(state_network);
				}
			}

			StateNetwork* score_network = loop_fold->curr_score_networks[f_index];
			score_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

			ActionNode* node = new ActionNode(state_network_target_is_local,
											  state_network_target_indexes,
											  state_networks,
											  score_network);
			new_nodes.push_back(node);
		}
	}

	vector<StateNetwork*> starting_state_networks;
	for (int i_index = 0; i_index < loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states; i_index++) {
		StateNetwork* state_network = loop_fold->curr_starting_state_networks[i_index];
		state_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);
		starting_state_networks.push_back(state_network);
	}

	StateNetwork* continue_score_network = loop_fold->curr_continue_score_network;
	continue_score_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

	StateNetwork* continue_misguess_network = loop_fold->curr_continue_misguess_network;
	continue_misguess_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

	StateNetwork* halt_score_network = loop_fold->curr_halt_score_network;
	halt_score_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

	StateNetwork* halt_misguess_network = loop_fold->curr_halt_misguess_network;
	halt_misguess_network->split_new_inner(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states);

	Scope* new_scope = new Scope(loop_fold->sum_inner_inputs+loop_fold->curr_num_new_inner_states,
								 loop_fold->num_local_states+loop_fold->num_input_states+loop_fold->curr_num_new_outer_states,
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
