#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	std::vector<bool> state_network_target_is_local;
	std::vector<int> state_network_target_indexes;
	std::vector<StateNetwork*> state_networks;

	ScoreNetwork* score_network;

	int next_node_id;	// TODO: set when adding to Scope?

	// TODO: don't explore if explore weight is below some threshold?

	// TODO: add batch surprise and seeding

	// TODO: context also has to have enough representation
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	// if doesn't match context, pass explore weight on
	// TODO: explore_action_sequences
	int explore_exit_depth;
	int explore_next_node_id;
	Fold* explore_fold;

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
						 std::vector<int>& scope_context,
						 std::vector<int>& node_context,
						 std::vector<int>& context_iter,
						 RunHelper& run_helper);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<NetworkHistory*> state_network_histories;
	NetworkHistory* score_network_history;


};

#endif /* ACTION_NODE_H */