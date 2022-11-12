/**
 * Branches start halfway in a node, but end after the end of a full node
 * Take outer local_state_vals and local_s_input_vals, and modify directly
 */

#ifndef BRANCH_H
#define BRANCH_H

#include <vector>

class Branch {
public:
	int num_states_pre_compress;	// (i.e., num_input)
	// for scope s_inputs, add all at start (initial SmallNetwork will be OK)
	// and don't change s_input
	int num_s_input;
	int num_output;

	std::vector<SmallNetwork*> score_networks;	// for initial, take score_network (potentially from inner scope)
	std::vector<Network*> compression_networks;	// take compression from inner as well
	std::vector<Scope*> branches;


};

#endif /* BRANCH_H */