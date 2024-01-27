#ifndef STATE_STATUS_H
#define STATE_STATUS_H

#include <map>
#include <set>

class PotentialScopeNode;
class Scope;
class StateNetwork;

class StateStatus {
public:
	double val;
	StateNetwork* last_network;

	std::set<int> involved_input;
	std::set<int> involved_local;
	std::map<PotentialScopeNode*, std::set<int>> involved_output;

	StateStatus();
	StateStatus(double val);

	void update_involved(std::set<int>& obs_involved_input,
						 std::set<int>& obs_involved_local,
						 std::map<PotentialScopeNode*, std::set<int>>& obs_involved_output);
};

#endif /* STATE_STATUS_H */