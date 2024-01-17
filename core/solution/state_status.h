#ifndef STATE_STATUS_H
#define STATE_STATUS_H

#include <map>
#include <set>

class Scope;
class StateNetwork;

class StateStatus {
public:
	double val;
	StateNetwork* last_network;

	/**
	 * - {input_indexes, local_indexes}
	 */
	std::map<Scope*, std::pair<std::set<int>,std::set<int>>> impacted_potential_scopes;

	StateStatus();
	StateStatus(double val);

	void update_impacted_potential_scopes(
		std::map<Scope*, std::pair<std::set<int>,std::set<int>>>& new_impacted_potential_scopes);
};

#endif /* STATE_STATUS_H */