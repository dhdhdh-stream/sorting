#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> target_indexes;
	std::vector<StateNetwork*> state_networks;

	ScoreNetwork* score_network;

	int next_node_id;

	double average_score;
	double score_variance;
	double average_misguess;
	double misguess_variance;

	double average_impact;

	// TODO: assign is_explore if run passed without any explore node being hit
	bool is_explore;
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_curr_try;
	double best_explore_surprise;
	AbstractExperiment* best_experiment;

	AbstractExperiment* curr_experiment;

	ActionNode(std::vector<int> target_indexes,
			   std::vector<StateNetwork*> state_networks,
			   ScoreNetwork* score_network,
			   int next_node_id,
			   double average_score,
			   double score_variance,
			   double average_misguess,
			   double misguess_variance,
			   double average_impact);
	ActionNode();
	~ActionNode();

	void activate(std::vector<double>& flat_vals,
				  std::vector<ForwardContextLayer>& context,
				  std::vector<std::vector<StateNetwork*>>*& experiment_scope_state_networks,
				  std::vector<ScoreNetwork*>*& experiment_scope_score_networks,
				  int& experiment_scope_distance,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
	void backprop(std::vector<BackwardContextLayer>& context,
				  double& scale_factor_error,
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

	AbstractExperimentHistory* experiment_history;

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