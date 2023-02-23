#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	std::vector<bool> state_network_target_is_local;
	std::vector<int> state_network_target_indexes;
	std::vector<Network*> state_networks;

	bool has_score_network;
	Network* score_network;

	int next_node_index;	// TODO: set when adding to Scope?

	// TODO: don't explore if explore weight is below some threshold?

	// TODO: to make sure balanced, pick context depth in advance, and try existing fold, or brand new explore, but don't try fold of less length
	std::vector<std::vector<int>> explore_scope_context;
	std::vector<std::vector<int>> explore_node_context;
	// TODO: explore_action_sequences
	std::vector<int> explore_next_indexes;	// next within outermost scope_context
	std::vector<Fold*> explore_folds;

	ActionNode(Scope* parent,
			   std::vector<bool> state_network_target_is_local,
			   std::vector<int> state_network_target_indexes,
			   std::vector<Network*> state_networks,
			   bool has_score_network,
			   Network* score_network);
	~ActionNode();

	int explore_activate(std::vector<double>& input_vals,
						 std::vector<double>& local_state_vals,
						 std::vector<std::vector<double>>& flat_vals,
						 double& predicted_score,
						 std::vector<int> scope_context,
						 std::vector<int> node_context,
						 RunStatus& run_status,
						 std::vector<AbstractNodeHistory*>& node_history);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<NetworkHistory*> state_network_histories;
	NetworkHistory* score_network_history;


};

#endif /* ACTION_NODE_H */