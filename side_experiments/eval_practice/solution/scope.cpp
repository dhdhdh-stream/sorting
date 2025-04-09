#include "scope.h"

#include "abstract_node.h"

using namespace std;

Scope::Scope() {
	this->node_counter = 0;
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

ScopeHistory::ScopeHistory(Scope* scope) {
	this->scope = scope;
}

ScopeHistory::~ScopeHistory() {
	for (map<int, AbstractNodeHistory*>::iterator it = this->node_histories.begin();
			it != this->node_histories.end(); it++) {
		delete it->second;
	}
}
