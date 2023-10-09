#ifndef STATE_STATUS_H
#define STATE_STATUS_H

class StateNetwork;

class StateStatus {
public:
	double val;
	StateNetwork* last_network;

	int last_updated;

	StateStatus() {
		this->val = 0.0;
		this->last_network = NULL;

		this->last_updated = -1;
	}

	StateStatus(double val) {
		this->val = val;
		this->last_network = NULL;

		this->last_updated = -1;
	}
};

#endif /* STATE_STATUS_H */