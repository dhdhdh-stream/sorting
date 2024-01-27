#include "state_status.h"

#include "scope.h"

using namespace std;

StateStatus::StateStatus() {
	this->val = 0.0;
	this->last_network = NULL;
}

StateStatus::StateStatus(double val) {
	this->val = val;
	this->last_network = NULL;
}

void StateStatus::update_involved(set<int>& obs_involved_input,
								  set<int>& obs_involved_local,
								  map<PotentialScopeNode*, set<int>>& obs_involved_output) {
	for (set<int>::iterator it = obs_involved_input.begin();
			it != obs_involved_input.end(); it++) {
		this->involved_input.insert(*it);
	}
	for (set<int>::iterator it = obs_involved_local.begin();
			it != obs_involved_local.end(); it++) {
		this->involved_local.insert(*it);
	}
	for (map<PotentialScopeNode*, set<int>>::iterator obs_it = obs_involved_output.begin();
			obs_it != obs_involved_output.end(); obs_it++) {
		map<PotentialScopeNode*, set<int>>::iterator local_it = this->involved_output.find(obs_it->first);
		if (local_it == this->involved_output.end()) {
			local_it = this->involved_output.insert({obs_it->first, set<int>()}).first;
		}
		for (set<int>::iterator index_it = obs_it->second.begin();
				index_it != obs_it->second.end(); index_it++) {
			local_it->second.insert(*index_it);
		}
	}
}
