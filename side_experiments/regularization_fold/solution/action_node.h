#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<StateNetwork*> state_networks;

	ScoreNetwork* score_network;


	bool is_explore;
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_curr_try;
	double best_explore_surprise;
	AbstractExperiment* best_experiment;

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;
	std::vector<double> starting_state_vals_snapshot;
	std::vector<StateNetworkHistory*> state_network_histories;
	std::vector<double> ending_state_vals_snapshot;
	ScoreNetworkHistory* score_network_history;
	double score_network_output;

	int experiment_context_index;
	std::vector<double> starting_new_state_vals_snapshot;
	std::vector<StateNetworkHistory*> new_state_network_histories;	// if zeroed, set to NULL
	std::vector<double> ending_new_state_vals_snapshot;
	ScoreNetwork* new_score_network_history;
	double new_score_network_output;

	std::vector<int> experiment_sequence_step_indexes;
	std::vector<std::vector<int>> input_vals_snapshots;
	std::vector<std::vector<StateNetworkHistory*>> input_state_network_histories;
};

#endif /* ACTION_NODE_H */