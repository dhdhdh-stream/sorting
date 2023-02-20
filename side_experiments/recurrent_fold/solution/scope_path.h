#ifndef SCOPE_PATH_H
#define SCOPE_PATH_H

#include "network.h"
#include "scope.h"

class Scope;
class ScopePath {
public:
	// TODO: think about removing inputs
	int num_input_states;	// also becomes output
	int num_local_states;

	int sequence_length;
	std::vector<bool> is_inner_scope;
	std::vector<Scope*> scopes;

	// if inner_scope expanded so there's no matching index, then pass NULL
	// copy val on start if not NULL, and copy val back on end
	std::vector<std::vector<bool>> inner_input_is_local;
	std::vector<std::vector<int>> inner_input_indexes;
	// std::vector<Network*> scope_scale_mod;

	std::vector<std::vector<bool>> state_network_target_is_local;
	std::vector<std::vector<int>> state_network_target_indexes;
	std::vector<std::vector<Network*>> state_networks;

	std::vector<bool> has_score_network;
	std::vector<Network*> score_networks;
	// score_network at end, on branch, initialize a new score network that will do nothing initially

	// TODO: add impacts

	// TODO: add measurements

	ScopePath(int num_input_states,
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
			  std::vector<Network*> score_networks);
	~ScopePath();
};

#endif /* SCOPE_PATH_H */