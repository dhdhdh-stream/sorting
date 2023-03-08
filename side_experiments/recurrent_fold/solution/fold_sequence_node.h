#ifndef FOLD_SEQUENCE_NODE_H
#define FOLD_SEQUENCE_NODE_H

class FoldSequenceNode : public AbstractNode {
public:
	Fold* fold;

	int next_node_id;

	// Note: don't backprop outer input errors into their scopes
	// potentially scopes that haven't been initialized yet
	// too complicated, and not significant?
};

#endif /* FOLD_SEQUENCE_NODE_H */