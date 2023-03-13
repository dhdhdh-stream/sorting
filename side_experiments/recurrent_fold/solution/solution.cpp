#include "solution.h"

#include "action_node.h"
#include "state_network.h"

using namespace std;

Solution::Solution() {
	this->average_score = 0.0;

	// starting action ACTION_START
	ActionNode* starting_node = new ActionNode(vector<bool>(),
											   vector<int>(),
											   vector<StateNetwork*>(),
											   new StateNetwork(1,
																0,
																0,
																0,
																0,
																20));
	this->scopes.push_back(new Scope(0,
									 0,
									 false,
									 NULL,
									 NULL,
									 vector<AbstractNode*>{starting_node}));
	this->scopes[0]->id = 0;

	this->max_depth = 1;
	this->depth_limit = 11;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}
