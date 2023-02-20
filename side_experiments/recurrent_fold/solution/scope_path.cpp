#include "scope_path.h"

using namespace std;

ScopePath::ScopePath(int num_input_states,
					 int num_local_states,
					 int sequence_length,
					 std::vector<bool> is_inner_scope,
					 std::vector<Scope*> scopes,
					 std::vector<std::vector<bool>> inner_input_is_local,
					 std::vector<std::vector<int>> inner_input_indexes,
					 std::vector<std::vector<bool>> state_network_target_is_local,
					 std::vector<std::vector<int>> state_network_target_indexes,
					 std::vector<std::vector<Network*>> state_networks,
					 std::vector<bool> has_score_network,
					 std::vector<Network*> score_networks) {
	this->num_input_states = num_input_states;
	this->num_local_states = num_local_states;
	this->sequence_length = sequence_length;
	this->is_inner_scope = is_inner_scope;
	this->scopes = scopes;
	this->inner_input_is_local = inner_input_is_local;
	this->inner_input_indexes = inner_input_indexes;
	this->state_network_target_is_local = state_network_target_is_local;
	this->state_network_target_indexes = state_network_target_indexes;
	this->state_networks = state_networks;
	this->has_score_network = has_score_network;
	this->score_networks = score_networks;
}

ScopePath::~ScopePath() {
	// TODO: switch to using scope dictionary
	for (int a_index = 0; a_index < this->sequence_length; a_index++) {
		if (this->scopes[a_index] != NULL) {
			delete this->scopes[a_index];
		}

		for (int s_index = 0; s_index < (int)this->state_networks[a_index].size(); s_index++) {
			delete this->state_networks[a_index][s_index];
		}

		if (this->score_networks[a_index] != NULL) {
			delete this->score_networks[a_index];
		}
	}
}
