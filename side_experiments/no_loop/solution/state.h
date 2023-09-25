#ifndef STATE_H
#define STATE_H

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	std::set<StateNetwork*> preceding_networks;
	double resolved_standard_deviation;

	Scale* scale;

	std::vector<AbstractNode*> nodes;
	/**
	 * - for tracking while still score state
	 */

};

class StateStatus {
public:
	double val;
	StateNetwork* last_network;

	StateStatus() {
		this->val = 0.0;
		this->last_network = NULL;
	}

	StateStatus(double val) {
		this->val = val;
		this->last_network = NULL;
	}
};

#endif /* STATE_H */