#include "action_node.h"

using namespace std;

ActionNode::ActionNode(int action,
					   vector<int> state_indexes,
					   vector<bool> state_xor_switch) {
	this->type = NODE_TYPE_ACTION;

	this->action = action;
	this->state_indexes = state_indexes;
	this->state_xor_switch = state_xor_switch;
}

ActionNode::ActionNode(ActionNode* original) {
	this->type = NODE_TYPE_ACTION;

	this->action = original->action;
	this->state_indexes = original->state_indexes;
	this->state_xor_switch = original->state_xor_switch;
}

ActionNode::~ActionNode() {
	// do nothing
}

void ActionNode::activate(int& curr_spot,
						  int& curr_0_index,
						  vector<int>& spots,
						  vector<bool>& switches,
						  int& num_actions) {
	if (this->action == ACTION_LEFT) {
		if (curr_spot != 0) {
			curr_spot--;
		}
	} else if (this->action == ACTION_RIGHT) {
		if (curr_spot != 9) {
			curr_spot++;
		}
	}

	num_actions++;

	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		if (this->state_indexes[s_index] == curr_0_index) {
			spots.push_back(curr_spot);
			switches.push_back(this->state_xor_switch[s_index]);
		}
	}
}

void ActionNode::fetch_context(vector<Scope*>& scope_context,
							   vector<int>& node_context,
							   int& curr_num_action,
							   int target_num_action) {
	curr_num_action++;
}
