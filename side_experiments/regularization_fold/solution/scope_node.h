#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	int inner_scope_id;
	std::vector<int> inner_input_indexes;
	std::vector<int> inner_input_target_indexes;
	/**
	 * - what networks to use in inner
	 *   - may not match outer original type
	 * - NULL if use outer original type
	 */
	std::vector<StateDefinition*> inner_input_types;
	std::vector<Transformation*> inner_input_transformations;
	Scale* scope_scale_mod;

	// TODO: update context before going inwards
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* inner_scope_history;
};

#endif /* SCOPE_NODE_H */