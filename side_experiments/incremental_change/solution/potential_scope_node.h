#ifndef POTENTIAL_SCOPE_NODE_H
#define POTENTIAL_SCOPE_NODE_H

class PotentialScopeNodeRun {
public:
	std::vector<std::vector<int>> possible_scope_contexts;
	std::vector<std::vector<int>> possible_node_contexts;


};

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

	/**
	 * - only worry about zeroing/removing state
	 *   - unlikely for potential scope node to be good enough without relevant state
	 */
	AbstractPotentialScopeNode* zero_state;
	AbstractPotentialScopeNode* remove_state;

};

class PotentialScopeNode : public AbstractPotentialScopeNode {
public:
	PotentialScopeNodeRun* run;

	int start_index;
	AbstractPotentialScopeNode* earlier_start;
	AbstractPotentialScopeNode* later_start;
	int end_index;
	AbstractPotentialScopeNode* earlier_end;
	AbstractPotentialScopeNode* later_end;

	std::vector<std::map<int, PotentialScopeNodeState>> start_state_mappings;
	std::vector<std::map<int, PotentialScopeNodeState>> end_state_mappings;


};

#endif /* POTENTIAL_SCOPE_NODE_H */