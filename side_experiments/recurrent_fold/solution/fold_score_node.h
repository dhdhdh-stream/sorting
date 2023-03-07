// TODO: is similar to branch node
// runs new outer state networks on outer scope
// call score_activate

// when replacing fold nodes, swap in place so indexes are preserved
// if is replace, can just pass through for now

// TODO: actually, maybe keep outer score networks here, and don't add them outside
// means less processing
// downside is the impact on predicted_score, and potential reuse

// Note: don't backprop outer input errors into their scopes
// potentially scopes that haven't been initialized yet
// too complicated, and not significant?

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