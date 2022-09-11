#include "action_dictionary.h"

#include <iostream>

#include "jump_scope.h"
#include "solution_node_action.h"
#include "solution_node_empty.h"

using namespace std;

void ActionDictionary::load(ifstream& save_file) {
	string num_actions_line;
	getline(save_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	this->actions.reserve(num_actions);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		string action_length_line;
		getline(save_file, action_length_line);
		int action_length = stoi(action_length_line);
		this->actions.push_back(vector<SolutionNode*>());
		this->actions[a_index].reserve(action_length);
		vector<int> scope_states;
		vector<int> scope_locations;
		scope_states.push_back(a_index);	// distinct from start_scope, which always starts with -1
		scope_locations.push_back(0);
		for (int n_index = 0; n_index < action_length; n_index++) {
			string node_type_line;
			getline(save_file, node_type_line);
			int node_type = stoi(node_type_line);
			SolutionNode* new_node;
			if (node_type == NODE_TYPE_EMPTY) {
				new_node = new SolutionNodeEmpty(scope_states,
												 scope_locations,
												 save_file);
			} else if (node_type == NODE_TYPE_ACTION) {
				new_node = new SolutionNodeAction(scope_states,
												  scope_locations,
												  save_file);
			} else {
				// node_type == NODE_TYPE_JUMP_SCOPE
				new_node = new JumpScope(scope_states,
										 scope_locations,
										 save_file);
			}
			this->actions[a_index].push_back(new_node);
		}
	}
}

void ActionDictionary::save(ofstream& save_file) {
	save_file << this->actions.size() << endl;
	for (int a_index = 0; a_index < (int)this->actions.size(); a_index++) {
		save_file << this->actions[a_index].size() << endl;
		vector<int> scope_states;
		vector<int> scope_locations;
		scope_states.push_back(a_index);	// distinct from start_scope, which always starts with -1
		scope_locations.push_back(0);
		for (int n_index = 0; n_index < (int)this->actions[a_index].size(); n_index++) {
			cout << this->actions[a_index][n_index]->node_type << endl;
			this->actions[a_index][n_index]->save(scope_states,
												  scope_locations,
												  save_file);
		}
	}
}
