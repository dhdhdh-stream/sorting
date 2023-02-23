#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

class ScopeNode : public AbstractNode {
public:
	Scope* inner_scope;
	std::vector<bool> inner_input_is_local;
	std::vector<int> inner_input_indexes;
	// std::vector<Network*> scope_scale_mod;

	bool has_score_network;
	Network* score_network;

	int next_node_index;	// TODO: set when adding to Scope?
	// may go elsewhere due to early exit, but no need to track (and don't explore branch there)

	// TODO: explore after inner scope too

	ScopeNode(Scope* parent,
			  Scope* inner_scope,
			  std::vector<bool> inner_input_is_local,
			  std::vector<int> inner_input_indexes,
			  bool has_score_network,
			  Network* score_network);
	~ScopeNode();

	int activate(std::vector<double>& input_vals,
				 std::vector<double>& local_state_vals,
				 std::vector<std::vector<double>>& flat_vals,
				 double& predicted_score,
				 int& early_exit_count,
				 int& early_exit_index);
};

#endif /* SCOPE_NODE_H */