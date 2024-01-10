#ifndef STATE_STATUS_H
#define STATE_STATUS_H

class StateNetwork;

class StateStatus {
public:
	double val;
	StateNetwork* last_network;

	std::map<Scope*, std::set<int>> impacted_potential_scopes;
	bool used;

	StateStatus();
	StateStatus(double val);

	void update_impacted_potential_scopes(
		std::map<Scope*, std::set<int>>& new_impacted_potential_scopes);
};

#endif /* STATE_STATUS_H */