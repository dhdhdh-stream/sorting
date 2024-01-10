// maybe only look for neighbors immediately
// after a while, will be changed such that can't easily adjust anymore

// so before adding anything, have a candidate
// - if find something completely new, then finalize candidate and swap

// to distribute, list all changes and mod

// actually, loops or slight increments don't matter enough
// - only worry about removing
// - i.e., only worry about abstraction

// repetition nearby is still good though

#ifndef POTENTIAL_SCOPE_NODE_H
#define POTENTIAL_SCOPE_NODE_H

class PotentialScopeNodeState {
public:
	bool added;

	bool is_init;

	/**
	 * - don't pull from or set temp state
	 * 
	 * - don't worry about separate potential scopes needing/sharing state that context doesn't have
	 */
	int depth;
	int index;
	bool is_pass_by_ref;
	/**
	 * - i.e., is output
	 * 
	 * - don't worry about same state outputting to multiple
	 */

	double init_val;
	double init_index_val;


};

class PotentialScopeNode {
public:
	int start_index;
	int end_index;

	std::vector<std::map<int, PotentialScopeNodeState>> start_state_mappings;
	std::vector<std::map<int, PotentialScopeNodeState>> end_state_mappings;


};

#endif /* POTENTIAL_SCOPE_NODE_H */