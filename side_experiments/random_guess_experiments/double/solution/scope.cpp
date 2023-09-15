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
					 int& curr_1_index,
					 vector<int>& spots_0,
					 vector<bool>& switches_0,
					 vector<int>& spots_1,
					 vector<bool>& switches_1,
					 int& num_actions) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->activate(curr_spot,
									   curr_0_index,
									   curr_1_index,
									   spots_0,
									   switches_0,
									   spots_1,
									   switches_1,
									   num_actions);
	}
}

void Scope::fetch_context(vector<Scope*>& scope_context,
						  vector<int>& node_context,
						  int& curr_num_action,
						  int target_num_action) {
	// special case if from very start, i.e., target_num_action == 0
	if (curr_num_action == target_num_action) {
		scope_context.insert(scope_context.begin(), this);
		node_context.insert(node_context.begin(), 0);

		return;
	}

	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->fetch_context(scope_context,
											node_context,
											curr_num_action,
											target_num_action);

		if (curr_num_action == target_num_action) {
			scope_context.insert(scope_context.begin(), this);
			node_context.insert(node_context.begin(), n_index);

			break;
		}
	}
}

void Scope::print(int& curr_spot,
				  int& curr_0_index,
				  int& curr_1_index) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->nodes[n_index]->print(curr_spot,
									curr_0_index,
									curr_1_index);
	}
}
