#include "explore_instance.h"

#include "abstract_node.h"
#include "scope.h"

using namespace std;

ExploreInstance::ExploreInstance() {
	this->new_scope = NULL;
}

ExploreInstance::~ExploreInstance() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int n_index = 0; n_index < (int)this->new_nodes.size(); n_index++) {
		delete this->new_nodes[n_index];
	}

	delete this->scope_history;
}
