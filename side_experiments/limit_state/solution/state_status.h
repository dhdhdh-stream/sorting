#ifndef STATE_STATUS_H
#define STATE_STATUS_H

class FullNetwork;

class StateStatus {
public:
	double val;
	double index;
	FullNetwork* last_network;

	StateStatus() {
		this->val = 0.0;
		this->index = 0.0;
		this->last_network = NULL;
	}

	StateStatus(double val,
				double index) {
		this->val = val;
		this->index = index;
		this->last_network = NULL;
	}

	bool operator==(const StateStatus& rhs) const {
		return this->val == rhs.val
			&& this->index == rhs.index
			&& this->last_network == rhs.last_network;
	}
};

#endif /* STATE_STATUS_H */