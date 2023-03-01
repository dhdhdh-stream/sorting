#ifndef FOLD_SEQUENCE_NODE_H
#define FOLD_SEQUENCE_NODE_H

class FoldSequenceNode : public AbstractNode {
public:
	Fold* fold;

	int next_node_id;
};

#endif /* FOLD_SEQUENCE_NODE_H */