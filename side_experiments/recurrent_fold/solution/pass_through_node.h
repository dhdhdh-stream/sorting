#ifndef PASS_THROUGH_NODE_H
#define PASS_THROUGH_NODE_H

class PassThroughNode : public AbstractNode {
public:
	int next_node_id;

	PassThroughNode(int next_node_id) {
		this->type = NODE_TYPE_PASS_THROUGH;
		this->next_node_id = next_node_id;
	}
};

#endif /* PASS_THROUGH_NODE_H */