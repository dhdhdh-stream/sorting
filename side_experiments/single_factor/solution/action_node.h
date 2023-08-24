#ifndef ACTION_NODE_H
#define ACTION_NODE_H

class ActionNode : public AbstractNode {
public:
	// Action action;

	/**
	 * - matches scope num_states
	 *   - NULL if no network needed
	 */
	std::vector<StateNetwork*> state_networks;


};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_snapshot;

	std::vector<StateNetworkHistory*> state_network_histories;


};

#endif /* ACTION_NODE_H */