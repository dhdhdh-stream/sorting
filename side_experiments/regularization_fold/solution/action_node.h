#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	// don't worry about type-specific x type-specific networks
	// when extending an object, don't give it type-specific state, so it will extend itself
	// so the same obs can cause matching changes to 2 types, but the 2 types will be independent
	std::map<ObjectDefinition*, std::vector<ObjectNetwork*>> networks;

	void activate(std::vector<double>& flat_vals,
				  std::vector<Object*>& state_vals,
				  double& predicted_score,
				  double& scale_factor,
				  RunHelper& run_helper,
				  ActionNodeHistory* history);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<ObjectNetworkHistory> network_histories;
	NetworkHistory* score_network_history;
	double score_network_update;

	double obs_snapshot;
	std::vector<Object> obj_vals_snapshot;	// starting

	// // for both pre and post
	// std::vector<NetworkHistory*> new_state_network_histories;
	// NetworkHistory* new_score_network_history;
	// double new_score_network_update;
};

#endif /* ACTION_NODE_H */