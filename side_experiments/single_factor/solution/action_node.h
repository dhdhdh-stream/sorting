#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> state_network_indexes;
	std::vector<StateNetwork*> state_networks;

	// TODO: learn simple score network for sequence
	std::vector<double> score_scales;

	int next_node_id;

	bool is_explore;
	std::vector<int> explore_scope_context;
	std::vector<int> explore_node_context;
	int explore_state;
	int explore_iter;
	ScoreNetwork* explore_score_network;
	ScoreNetwork* explore_misguess_network;
	double explore_best_surprise;
	AbstractExperiment* explore_best_experiment;

	AbstractExperiment* experiment;

};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_snapshot;

	std::vector<StateNetworkHistory*> state_network_histories;

	AbstractExperimentHistory* experiment_history;

	std::vector<double> state_snapshot;

};

#endif /* ACTION_NODE_H */