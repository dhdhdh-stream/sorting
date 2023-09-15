#include "action_node.h"

#include <iostream>

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
						  int& curr_1_index,
						  vector<int>& spots_0,
						  vector<bool>& switches_0,
						  vector<int>& spots_1,
						  vector<bool>& switches_1,
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
			spots_0.push_back(curr_spot);
			switches_0.push_back(this->state_xor_switch[s_index]);
		}

		if (this->state_indexes[s_index] == curr_1_index) {
			spots_1.push_back(curr_spot);
			switches_1.push_back(this->state_xor_switch[s_index]);
		}
	}
}

void ActionNode::fetch_context(vector<Scope*>& scope_context,
							   vector<int>& node_context,
							   int& curr_num_action,
							   int target_num_action) {
	curr_num_action++;
}

void ActionNode::print(int& curr_spot,
					   int& curr_0_index,
					   int& curr_1_index) {
	if (this->action == ACTION_LEFT) {
		if (curr_spot != 0) {
			curr_spot--;
		}
	} else if (this->action == ACTION_RIGHT) {
		if (curr_spot != 9) {
			curr_spot++;
		}
	}

	if (this->action == ACTION_LEFT) {
		cout << "LEFT" << endl;
	} else if (this->action == ACTION_STAY) {
		cout << "STAY" << endl;
	} else {
		cout << "RIGHT" << endl;
	}

	for (int s_index = 0; s_index < (int)this->state_indexes.size(); s_index++) {
		if (this->state_indexes[s_index] == curr_0_index) {
			cout << "spot_0 " << curr_spot << endl;
			cout << "curr_0_index " << curr_0_index << endl;
		}

		if (this->state_indexes[s_index] == curr_1_index) {
			cout << "spot_1 " << curr_spot << endl;
			cout << "curr_1_index " << curr_1_index << endl;
		}
	}
}
