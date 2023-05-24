#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:

	int inner_scope_id;
	std::vector<int> inner_input_indexes;
	std::vector<int> inner_input_target_indexes;
	Scale* scope_scale_mod;

};

#endif /* SCOPE_NODE_H */