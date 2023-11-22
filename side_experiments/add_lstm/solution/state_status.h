#ifndef STATE_STATUS_H
#define STATE_STATUS_H

class StateNetwork;

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

	bool operator==(const StateStatus& rhs) const {
		return this->val == rhs.val && this->last_network == rhs.last_network;
	}
};

#endif /* STATE_STATUS_H */