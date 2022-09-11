#include "candidate_start_branch.h"

#include "definitions.h"
#include "jump_scope.h"

using namespace std;

CandidateStartBranch::CandidateStartBranch(StartScope* start_scope,
										   double branch_percent,
										   double score_increase,
										   int jump_end_non_inclusive_index,
										   vector<SolutionNode*> explore_path,
										   Network* small_jump_score_network,
										   Network* small_no_jump_score_network) {
	this->type = CANDIDATE_START_BRANCH;

	this->start_scope = start_scope;
	this->branch_percent = branch_percent;
	this->score_increase = score_increase;
	this->jump_end_non_inclusive_index = jump_end_non_inclusive_index;
	this->explore_path = explore_path;
	this->small_jump_score_network = small_jump_score_network;
	this->small_no_jump_score_network = small_no_jump_score_network;
}

void CandidateStartBranch::apply() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		this->explore_path[n_index]->set_is_temp_node(false);
	}

	if (this->explore_path[0]->node_type != NODE_TYPE_EMPTY) {
		vector<SolutionNode*> explore_path_deep_copy;
		for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
			explore_path_deep_copy.push_back(this->explore_path[n_index]->deep_copy(1));
		}
		action_dictionary->actions.push_back(explore_path_deep_copy);
	}

	vector<int> start_scope_local_state_sizes;
	start_scope_local_state_sizes.push_back(0);
	JumpScope* new_scope = new JumpScope(start_scope_local_state_sizes, 0);

	vector<SolutionNode*> original_path_child;
	for (int n_index = 0; n_index < this->jump_end_non_inclusive_index; n_index++) {
		original_path_child.push_back(this->start_scope->path[n_index]);
	}
	new_scope->children_nodes.push_back(original_path_child);
	new_scope->children_nodes.push_back(this->explore_path);

	if (this->jump_end_non_inclusive_index >= (int)this->start_scope->path.size()) {
		new_scope->next = this->start_scope;
	} else {
		new_scope->next = this->start_scope->path[this->jump_end_non_inclusive_index];
	}
	this->start_scope->path.erase(this->start_scope->path.begin(),
		this->start_scope->path.begin() + this->jump_end_non_inclusive_index);
	this->start_scope->path.insert(this->start_scope->path.begin(), new_scope);
	new_scope->parent_scope = this->start_scope;
	new_scope->scope_location = -1;
	new_scope->scope_child_index = -1;
	for (int n_index = 0; n_index < (int)this->start_scope->path.size(); n_index++) {
		this->start_scope->path[n_index]->scope_node_index = n_index;
	}

	for (int n_index = 0; n_index < (int)new_scope->children_nodes[0].size(); n_index++) {
		new_scope->children_nodes[0][n_index]->parent_scope = new_scope;
		new_scope->children_nodes[0][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
		new_scope->children_nodes[0][n_index]->scope_child_index = 0;
		new_scope->children_nodes[0][n_index]->scope_node_index = n_index;
	}
	if (new_scope->children_nodes[0].size() > 0) {
		new_scope->children_nodes[0][new_scope->children_nodes[0].size()-1]->next = new_scope;
	}
	for (int n_index = 0; n_index < (int)new_scope->children_nodes[0].size(); n_index++) {
		new_scope->children_nodes[0][n_index]->insert_scope((int)new_scope->local_state_sizes.size(), 0);
	}

	for (int n_index = 0; n_index < (int)new_scope->children_nodes[1].size(); n_index++) {
		new_scope->children_nodes[1][n_index]->parent_scope = new_scope;
		new_scope->children_nodes[1][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
		new_scope->children_nodes[1][n_index]->scope_child_index = 1;
		new_scope->children_nodes[1][n_index]->scope_node_index = n_index;
	}
	new_scope->children_nodes[1][new_scope->children_nodes[1].size()-1]->next = new_scope;
	for (int n_index = 0; n_index < (int)new_scope->children_nodes[1].size(); n_index++) {
		new_scope->children_nodes[1][n_index]->insert_scope((int)new_scope->local_state_sizes.size(), 0);
	}

	new_scope->children_score_networks.push_back(this->small_no_jump_score_network);
	new_scope->children_score_networks.push_back(this->small_jump_score_network);

	vector<SolutionNode*> scope_action;
	scope_action.push_back(new_scope);
	action_dictionary->actions.push_back(scope_action);
}

void CandidateStartBranch::clean() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}

	delete this->small_jump_score_network;
	delete this->small_no_jump_score_network;
}
