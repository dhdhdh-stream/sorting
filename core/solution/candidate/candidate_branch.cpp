#include "candidate_replace.h"

using namespace std;

CandidateBranch::CandidateBranch(ExploreNode* explore_node,
								 double branch_percent,
								 double score_increase,
								 int parent_jump_scope_start_non_inclusive_index,
								 int parent_jump_end_non_inclusive_index,
								 int new_state_size,
								 std::vector<SolutionNode*> explore_path,
								 Network* small_jump_score_network,
								 Network* small_no_jump_score_network) {
	this->type = CANDIDATE_BRANCH;

	this->explore_node = explore_node;
	this->branch_percent = branch_percent;
	this->score_increase = score_increase;
	this->parent_jump_scope_start_non_inclusive_index = parent_jump_scope_start_non_inclusive_index;
	this->parent_jump_end_non_inclusive_index = parent_jump_end_non_inclusive_index;
	this->new_state_size = new_state_size;
	this->explore_path = explore_path;
	this->small_jump_score_network = small_jump_score_network;
	this->small_no_jump_score_network = small_no_jump_score_network;
}

void CandidateBranch::apply() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		this->explore_path[n_index]->set_is_temp_node(false);
	}

	if (this->explore_node[0]->node_type != NODE_TYPE_EMPTY) {
		vector<SolutionNode*> explore_path_deep_copy;
		for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
			explore_path_deep_copy.push_back(this->explore_path[n_index]->deep_copy(this->explore_node->local_state_sizes.size()));
		}
		action_dictionary->actions.push_back(explore_path_deep_copy);
	}

	JumpScope* new_scope = new JumpScope(this->explore_node->local_state_sizes,
										 this->new_state_size);

	if (this->explore_node->parent_scope->node_type == NODE_TYPE_START_SCOPE) {
		StartScope* parent_start = (StartScope*)this->explore_node->parent_scope;

		for (int n_index = this->parent_jump_scope_start_non_inclusive_index+1; n_index <= this->explore_node->scope_node_index; n_index++) {
			new_scope->top_path.push_back(parent_start->path[n_index]);
		}
		vector<SolutionNode*> original_path_child;
		for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
			original_path_child.push_back(parent_start->path[n_index]);
		}
		new_scope->children_nodes.push_back(original_path_child);
		new_scope->children_nodes.push_back(this->explore_path);

		if (this->parent_jump_scope_start_non_inclusive_index != -1) {
			parent_start->path[this->parent_jump_scope_start_non_inclusive_index]->next = new_scope;
		}
		if (this->parent_jump_end_non_inclusive_index >= parent_start->path.size()) {
			new_scope->next = parent_start;
		} else {
			new_scope->next = parent_start->path[this->parent_jump_end_non_inclusive_index];
		}
		parent_start->path.erase(parent_start->path.begin() + this->parent_jump_scope_start_non_inclusive_index + 1,
			parent_start->path.begin() + this->parent_jump_end_non_inclusive_index);
		parent_start->path.insert(parent_start->path.begin() + this->parent_jump_scope_start_non_inclusive_index + 1, new_scope);
		new_scope->parent_scope = parent_start;
		new_scope->scope_location = -1;
		new_scope->scope_child_index = -1;
		for (int n_index = 0; n_index < (int)parent_start->path.size(); n_index++) {
			parent_start->path[n_index]->scope_node_index = n_index;
		}
	} else if (this->explore_node->parent_scope->node_type == NODE_TYPE_JUMP_SCOPE) {
		JumpScope* parent_jump = (JumpScope*)this->explore_node->parent_scope;
		if (this->explore_node->scope_location == SCOPE_LOCATION_TOP) {
			for (int n_index = this->parent_jump_scope_start_non_inclusive_index+1; n_index <= this->explore_node->scope_node_index; n_index++) {
				new_scope->top_path.push_back(parent_jump->top_path[n_index]);
			}
			vector<SolutionNode*> original_path_child;
			for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
				original_path_child.push_back(parent_jump->top_path[n_index]);
			}
			new_scope->children_nodes.push_back(original_path_child);
			new_scope->children_nodes.push_back(this->explore_path);

			if (this->parent_jump_scope_start_non_inclusive_index != -1) {
				parent_jump->top_path[this->parent_jump_scope_start_non_inclusive_index]->next = new_scope;
			}
			if (this->parent_jump_end_non_inclusive_index >= parent_start->path.size()) {
				new_scope->next = parent_jump;
			} else {
				new_scope->next = parent_jump->top_path[this->parent_jump_end_non_inclusive_index];
			}
			parent_jump->top_path.erase(parent_jump->top_path.begin() + this->parent_jump_scope_start_non_inclusive_index + 1,
				parent_jump->top_path.begin() + this->parent_jump_end_non_inclusive_index);
			parent_jump->top_path.insert(parent_jump->top_path.begin() + this->parent_jump_scope_start_non_inclusive_index + 1, new_scope);
			new_scope->parent_scope = parent_jump;
			new_scope->scope_location = SCOPE_LOCATION_TOP;
			new_scope->scope_child_index = -1;
			for (int n_index = 0; n_index < (int)parent_jump->top_path.size(); n_index++) {
				parent_jump->top_path[n_index]->scope_node_index = n_index;
			}
		} else {
			// this->explore_node->scope_location == SCOPE_LOCATION_BRANCH
			for (int n_index = this->parent_jump_scope_start_non_inclusive_index+1; n_index <= this->explore_node->scope_node_index; n_index++) {
				new_scope->top_path.push_back(parent_jump->children_nodes[explore_node->scope_child_index][n_index]);
			}
			vector<SolutionNode*> original_path_child;
			for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
				original_path_child.push_back(parent_jump->children_nodes[explore_node->scope_child_index][n_index]);
			}
			new_scope->children_nodes.push_back(original_path_child);
			new_scope->children_nodes.push_back(this->explore_path);

			if (this->parent_jump_scope_start_non_inclusive_index != -1) {
				parent_jump->children_nodes[explore_node->scope_child_index][
					this->parent_jump_scope_start_non_inclusive_index]->next = new_scope;
			}
			if (this->parent_jump_end_non_inclusive_index >= parent_jump->children_nodes[explore_node->scope_child_index].size()) {
				new_scope->next = parent_jump;
			} else {
				new_scope->next = parent_jump->children_nodes[explore_node->scope_child_index][this->parent_jump_end_non_inclusive_index];
			}
			parent_jump->children_nodes[explore_node->scope_child_index].erase(
				parent_jump->children_nodes[explore_node->scope_child_index].begin()
					+ this->parent_jump_scope_start_non_inclusive_index + 1,
				parent_jump->children_nodes[explore_node->scope_child_index].begin()
					+ this->parent_jump_end_non_inclusive_index);
			parent_jump->children_nodes[explore_node->scope_child_index].insert(
				parent_jump->children_nodes[explore_node->scope_child_index].begin(),
					+ this->parent_jump_end_non_inclusive_index,
				new_scope);
			new_scope->parent_scope = parent_jump;
			new_scope->scope_location = SCOPE_LOCATION_BRANCH;
			new_scope->scope_child_index = explore_node->scope_child_index;
			for (int n_index = 0; n_index < (int)parent_jump->children_nodes[explore_node->scope_child_index].size(); n_index++) {
				parent_jump->children_nodes[explore_node->scope_child_index][n_index]->scope_node_index = n_index;
			}
		}
	}

	for (int n_index = 0; n_index < (int)new_scope->top_path.size(); n_index++) {
		new_scope->top_path[n_index]->parent_scope = new_scope;
		new_scope->top_path[n_index]->scope_location = SCOPE_LOCATION_TOP;
		new_scope->top_path[n_index]->scope_child_index = -1;
		new_scope->top_path[n_index]->scope_node_index = n_index;
	}
	if (new_scope->top_path.size() > 0) {
		new_scope->top_path[new_scope->top_path.size()-1]->next = new_scope;
	}
	for (int n_index = 0; n_index < (int)new_scope->top_path.size(); n_index++) {
		new_scope->top_path[n_index]->insert_scope(new_scope->local_state_sizes.size(),
												   new_scope->num_states);
	}
	for (int n_index = 0; n_index < (int)this->existing_nodes.size(); n_index++) {
		if (this->existing_nodes[n_index]->node_type == NODE_TYPE_EMPTY) {
			SolutionNodeEmpty* node_empty = (SolutionNodeEmpty*)this->existing_nodes[n_index];
			node_empty->state_networks[new_scope->local_state_sizes.size()] = this->new_state_networks[n_index];
		} else {
			// this->existing_nodes[n_index]->node_type == NODE_TYPE_ACTION
			SolutionNodeAction* node_action = (SolutionNodeAction*)this->existing_nodes[n_index];
			node_action->state_networks[new_scope->local_state_sizes.size()] = this->new_state_networks[n_index];
		}
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
		new_scope->children_nodes[0][n_index]->insert_scope(new_scope->local_state_sizes.size(), 0);
	}

	for (int n_index = 0; n_index < (int)new_scope->children_nodes[1].size(); n_index++) {
		new_scope->children_nodes[1][n_index]->parent_scope = new_scope;
		new_scope->children_nodes[1][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
		new_scope->children_nodes[1][n_index]->scope_child_index = 1;
		new_scope->children_nodes[1][n_index]->scope_node_index = n_index;
	}
	new_scope->children_nodes[1][new_scope->children_nodes[1].size()-1]->next = new_scope;
	for (int n_index = 0; n_index < (int)new_scope->children_nodes[1].size(); n_index++) {
		new_scope->children_nodes[1][n_index]->insert_scope(new_scope->local_state_sizes.size(), 0);
	}

	new_scope->children_score_networks.push_back(this->small_no_jump_score_network);
	new_scope->children_score_networks.push_back(this->small_jump_score_network);

	vector<SolutionNode*> scope_action;
	scope_action.push_back(new_scope);
	action_dictionary->actions.push_back(scope_action);
}

void CandidateBranch::clean() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}

	delete this->small_jump_score_network;
	delete this->small_no_jump_score_network;

	for (int n_index = 0; n_index < (int)this->new_state_networks.size(); n_index++) {
		delete this->new_state_networks[n_index];
	}
}
