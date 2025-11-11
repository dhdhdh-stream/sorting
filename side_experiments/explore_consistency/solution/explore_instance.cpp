#include "explore_instance.h"

#include <iostream>
#include <sstream>

#include "abstract_node.h"
#include "constants.h"
#include "explore_experiment.h"
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

void ExploreInstance::print() {
	stringstream ss;
	ss << "this->experiment->scope_context->id: " << this->experiment->scope_context->id << "; ";
	ss << "this->experiment->node_context->id: " << this->experiment->node_context->id << "; ";
	ss << "this->experiment->is_branch: " << this->experiment->is_branch << "; ";
	ss << "new explore path:";
	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			ss << " " << this->actions[s_index];
		} else {
			ss << " E" << this->scopes[s_index]->id;
		}
	}
	ss << "; ";

	if (this->exit_next_node == NULL) {
		ss << "this->exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->exit_next_node->id: " << this->exit_next_node->id << "; ";
	}

	cout << ss.str() << endl;
}
