#include "state_status.h"

using namespace std;

StateStatus::StateStatus() {
	this->val = 0.0;
	this->last_network = NULL;
}

StateStatus::StateStatus(double val) {
	this->val = val;
	this->last_network = NULL;
}

void StateStatus::update_impacted_potential_scopes(
		map<Scope*, set<int>>& new_impacted_potential_scopes) {
	for (map<Scope*, set<int>>::iterator new_it = new_impacted_potential_scopes.begin();
			new_it != new_impacted_potential_scopes.end(); new_it++) {
		map<Scope*, set<int>>::iterator curr_it = this->impacted_potential_scopes.find(new_it->first);
		if (curr_it == this->impacted_potential_scopes.end()) {
			curr_it = this->impacted_potential_scopes.insert({new_it->first, set<int>()}).first;
		}
		for (set<int>::iterator index_it = new_it->second.begin();
				index_it != new_it->second.end(); index_it++) {
			curr_it->second.insert(*index_it);
		}
	}
}
