#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:

	int inner_scope_id;
	std::vector<int> inner_input_indexes;
	std::vector<int> inner_input_target_indexes;
	Scale* scope_scale_mod;

	// TODO: post networks take input from inner as well
	// - run even on early exit

	// TODO: add transformations
	// - and types to transform to?

	// TODO: update context before going inwards
};

#endif /* SCOPE_NODE_H */