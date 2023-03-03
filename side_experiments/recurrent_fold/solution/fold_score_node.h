// TODO: is similar to branch node
// runs new outer state networks on outer scope
// call score_activate

// when replacing fold nodes, swap in place so indexes are preserved
// if is replace, can just pass through for now

#ifndef FOLD_SCORE_NODE_H
#define FOLD_SCORE_NODE_H

class FoldScoreNode : public AbstractNode {
public:
	StateNetwork* existing_score_network;
	int existing_next_node_id;

	Fold* fold;

	vector<int> fold_scope_context;
	vector<int> fold_node_context;
	int fold_exit_depth;
	int fold_next_node_id;

};

#endif /* FOLD_SCORE_NODE_H */