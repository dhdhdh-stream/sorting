#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>

#include "state_status.h"

class BranchNode;
class ScopeHistory;
class State;

class ContextLayer {
public:
	int scope_id;
	int node_id;

	/**
	 * - for inputs, initialize by adding to state_vals at scope node
	 *   - so if empty when find, then ignore
	 *   - set input_init_vals to 0.0 at top
	 * - for local, always initialized
	 */
	std::map<int, StateStatus> input_state_vals;
	std::map<int, StateStatus> local_state_vals;

	std::vector<BranchNode*> added_recursion_protection_flags;

	std::map<State*, StateStatus> score_state_vals;
	std::map<State*, StateStatus> experiment_score_state_vals;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */