#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	std::vector<std::map<StateDefinition*, Network*>> state_networks;
	// set to NULL if tried and decided network was unneeded

	Network* score_network;



	void activate(std::vector<double>& flat_vals,
				  std::vector<double>& state_vals,
				  std::vector<StateDefinition*>& state_types,	// set to NULL if not initialized
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	double obs_snapshot;
	std::vector<double> starting_state_vals_snapshot;
	std::vector<double> ending_state_vals_snapshot;
	double score_network_output;

	std::vector<double> starting_new_state_vals_snapshot;
	std::vector<bool> network_zeroed;
	std::vector<double> ending_new_state_vals_snapshot;
};

#endif /* ACTION_NODE_H */