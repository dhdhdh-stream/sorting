#ifndef STATE_H
#define STATE_H

class State {
public:
	int id;

	std::vector<StateNetwork*> networks;

	std::set<StateNetwork*> resolved_networks;
	double resolved_standard_deviation;

	Scale* scale;

	std::vector<AbstractNode*> nodes;
	/**
	 * - for tracking while still score state
	 * 
	 * - may be duplicates if multiples obs from same node
	 */

	State();
	~State() {
		for (int n_index = 0; n_index < (int)this->networks.size(); n_index++) {
			delete this->networks[n_index];
		}

		delete this->scale;
	};
};

#endif /* STATE_H */