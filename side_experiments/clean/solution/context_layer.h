#ifndef CONTEXT_LAYER_H
#define CONTEXT_LAYER_H

#include <map>

#include "state_status.h"

class Scope;
class ScopeHistory;
class ScopeNode;
class State;

class ContextLayer {
public:
	Scope* scope;
	ScopeNode* node;

	/**
	 * - for inputs, initialize by adding to state_vals at scope node
	 *   - so if empty when find, then ignore
	 *   - set input_init_vals to 0.0 at top
	 * - for local, always initialized
	 */
	std::map<int, StateStatus> input_state_vals;
	std::map<int, StateStatus> local_state_vals;

	std::map<State*, StateStatus> temp_state_vals;

	ScopeHistory* scope_history;
};

#endif /* CONTEXT_LAYER_H */