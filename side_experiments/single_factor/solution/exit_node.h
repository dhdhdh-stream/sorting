#ifndef EXIT_NODE_H
#define EXIT_NODE_H

#include "abstract_node.h"

class ExitNode : public AbstractNode {
public:
	int exit_depth;		// can be 0
	int exit_node_id;

	/**
	 * - only needed for scope_context.size()
	 */
	std::vector<int> scope_context;
	std::vector<int> node_context;
	/**
	 * - index 0 is global
	 */
	std::vector<int> target_layers;
	std::vector<int> target_indexes;
	std::vector<double> weight_mods;


};

#endif /* EXIT_NODE_H */