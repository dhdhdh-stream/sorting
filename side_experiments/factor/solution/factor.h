#ifndef FACTOR_H
#define FACTOR_H

class Factor {
public:
	ObsNode* parent;
	int index;

	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> inputs;
	Network* network;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<int>> input_node_context_ids;

};

#endif /* FACTOR_H */