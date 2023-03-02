/**
 * Notes:
 * - no matter the actual topology, simply iterate through node IDs
 *   - but this means need to iterate through every node
 */ 

#include "fold.h"

using namespace std;

const int CLEAN_OUTER_CONTEXT_NEXT_SCOPE = 0;
const int CLEAN_OUTER_CONTEXT_DONE = 1;

int clean_outer_index_curr_scope_id() {
	Scope* curr_scope = solution->scopes[this->scope_context[this->clean_outer_context_index]];

	for (int i_index = 0; i_index < (int)this->clean_outer_index.size()-1; i_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->clean_outer_index[i_index]];
		curr_scope = solution->scopes[scope_node->inner_scope_id];
	}

	return curr_scope->id;
}

int clean_outer_index_next() {
	while (true) {
		this->clean_outer_index.back()++;

		Scope* curr_scope = solution->scopes[clean_outer_index_curr_scope_id()];
		if (this->clean_outer_index.back() >= curr_scope->nodes.size()) {
			this->clean_outer_index.pop_back();
			if (this->clean_outer_index.size() == 0) {
				return CLEAN_OUTER_CONTEXT_DONE;
			}
			// else continue
		} else {
			if (curr_scope->nodes[this->clean_outer_index.back()]->type == NODE_TYPE_INNER_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->clean_outer_index.back()];

				bool is_scope_context = false;
				for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
					if (scope_context[c_index] == scope_node->inner_scope_id) {
						is_scope_context = true;
						break;
					}
				}
				if (is_scope_context) {
					continue;
				}

				map<int, bool>::iterator checked_it = this->curr_outer_scopes_needed.find(scope_node->inner_scope_id);
				if (checked_it == this->curr_outer_scopes_needed.end()) {
					map<int, vector<vector<StateNetwork*>>>::iterator network_it =
						this->curr_outer_state_networks.find(scope_node->inner_scope_id);
					if (network_it != this->curr_outer_state_networks.end()) {
						return CLEAN_OUTER_CONTEXT_NEXT_SCOPE;
						// TODO: if needed, then go in, and extend clean_outer_index
					}
					// else continue
				}
				// else continue
			}
			// else continue
		}
	}
}
