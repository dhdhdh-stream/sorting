#ifndef FOLD_NODE_H
#define FOLD_NODE_H

class FoldNode : public AbstractNode {
public:
	Fold* fold;

	int early_exit_count;
	int next_node_index;	// TODO: shared with early exit

	// TODO: uses outer state for sequence
};

#endif /* FOLD_NODE_H */