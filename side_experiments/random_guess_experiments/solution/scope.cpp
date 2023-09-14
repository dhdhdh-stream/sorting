#include "scope.h"

#include "action_node.h"
#include "scope_node.h"

using namespace std;

Scope::Scope(vector<AbstractNode*> nodes) {
	this->nodes = nodes;
}

Scope::Scope(Scope* original) {
	for (int n_index = 0; n_index < (int)original->nodes.size(); n_index++) {
		if (original->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)original->nodes[n_index];
			this->nodes.push_back(new ActionNode(action_node));
		} else {
			ScopeNode* scope_node = (ScopeNode*)original->nodes[n_index];
			this->nodes.push_back(new ScopeNode(scope_node));
		}
	}
}

Scope::~Scope() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		delete this->nodes[n_index];
	}
}

void Scope::activate(int& curr_spot,
					 int& curr_0_index,
					 vector<int>& spots,
					 vector<bool>& switches,
					 int& num_actions) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->activate(curr_spot,
									   curr_0_index,
									   spots,
									   switches,
									   num_actions);
	}
}

void Scope::fetch_context(vector<Scope*>& scope_context,
						  vector<int>& node_context,
						  int& curr_num_action,
						  int target_num_action) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (curr_num_action == target_num_action) {
			scope_context.insert(scope_context.begin(), this);
			node_context.insert(node_context.begin(), n_index);

			break;
		}

		this->nodes[n_index]->fetch_context(scope_context,
											node_context,
											curr_num_action,
											target_num_action);
	}
}
