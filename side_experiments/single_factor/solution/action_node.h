#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<int> state_network_indexes;
	std::vector<StateNetwork*> state_networks;

	int next_node_id;

	bool is_explore;
	int explore_curr_try;
	double explore_best_surprise;
	AbstractExperiment* explore_best_experiment;

	AbstractExperiment* experiment;

};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_snapshot;

	std::vector<int> state_network_indexes;
	std::vector<StateNetworkHistory*> state_network_histories;

	AbstractExperimentHistory* experiment_history;

};

#endif /* ACTION_NODE_H */