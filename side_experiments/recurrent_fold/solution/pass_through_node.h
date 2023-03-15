#ifndef PASS_THROUGH_NODE_H
#define PASS_THROUGH_NODE_H

#include "abstract_node.h"

class PassThroughNode : public AbstractNode {
public:
	int next_node_id;

	PassThroughNode(int next_node_id);
	PassThroughNode(std::ifstream& input_file,
					int scope_id,
					int scope_index);

	void save(std::ofstream& output_file,
			  int scope_id,
			  int scope_index);
};

#endif /* PASS_THROUGH_NODE_H */