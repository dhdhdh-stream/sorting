#include "candidate_replace.h"

using namespace std;

CandidateReplace::CandidateReplace(ExploreNode* explore_node,
								   int replace_type,
								   double score_increase,
								   double info_gain,
								   int parent_jump_scope_start_non_inclusive_index,
								   int parent_jump_end_non_inclusive_index,
								   std::vector<SolutionNode*> explore_path) {
	this->type = CANDIDATE_REPLACE;

	this->explore_node = explore_node;
	this->replace_type = replace_type;
	this->score_increase = score_increase;
	this->info_gain = info_gain;
	this->parent_jump_scope_start_non_inclusive_index = parent_jump_scope_start_non_inclusive_index;
	this->parent_jump_end_non_inclusive_index = parent_jump_end_non_inclusive_index;
	this->explore_path = explore_path;
}

void CandidateReplace::apply() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		this->explore_path[n_index]->set_is_temp_node(false);
	}

	if (this->explore_node.size() > 1
			|| this->explore_node[0]->node_type != NODE_TYPE_EMPTY) {
		vector<SolutionNode*> explore_path_deep_copy;
		for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
			explore_path_deep_copy.push_back(this->explore_path[n_index]->deep_copy(this->explore_node->local_state_sizes.size()));
		}
		action_dictionary->actions.push_back(explore_path_deep_copy);
	}

	if (this->explore_node->parent_scope->node_type == NODE_TYPE_START_SCOPE) {
		StartScope* parent_start = (StartScope*)this->explore_node->parent_scope;
		for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
			delete parent_start->path[n_index];
		}
		parent_start->path.erase(parent_start->path.begin() + this->explore_node->scope_node_index+1,
			parent_start->path.begin() + this->parent_jump_end_non_inclusive_index);

		parent_start->path.insert(parent_start->path.begin() + this->explore_node->scope_node_index+1,
			this->explore_path.begin(), this->explore_path.end());
		this->explore_node->next = this->explore_path[0];
		if (this->parent_jump_end_non_inclusive_index >= (int)parent_start->path.size()) {
			this->explore_path[this->explore_path.size()-1]->next = parent_start;
		} else {
			this->explore_path[this->explore_path.size()-1]->next = parent_start->path[this->parent_jump_end_non_inclusive_index];
		}

		for (int n_index = 0; n_index < (int)parent_start->path.size(); n_index++) {
			parent_start->path[n_index]->parent_scope = parent_start;
			parent_start->path[n_index]->scope_location = -1;
			parent_start->path[n_index]->scope_child_index = -1;
			parent_start->path[n_index]->scope_node_index = n_index;
		}
	} else if (this->explore_node->parent_scope->node_type == NODE_TYPE_JUMP_SCOPE) {
		JumpScope* parent_jump = (JumpScope*)this->explore_node->parent_scope;
		if (this->explore_node->scope_location == SCOPE_LOCATION_TOP) {
			for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
				delete parent_jump->top_path[n_index];
			}
			parent_jump->top_path.erase(parent_jump->top_path.begin() + this->explore_node->scope_node_index+1,
				parent_jump->top_path.begin() + this->parent_jump_end_non_inclusive_index);

			parent_jump->top_path.insert(parent_jump->top_path.begin() + this->explore_node->scope_node_index+1,
				this->explore_path.begin(), this->explore_path.end());
			this->explore_node->next = this->explore_path[0];
			if (this->parent_jump_end_non_inclusive_index >= (int)parent_jump->top_path.size()) {
				this->explore_path[this->explore_path.size()-1]->next = parent_jump;
			} else {
				this->explore_path[this->explore_path.size()-1]->next = parent_jump->top_path[this->parent_jump_end_non_inclusive_index];
			}

			for (int n_index = 0; n_index < (int)parent_jump->top_path.size(); n_index++) {
				parent_jump->top_path[n_index]->parent_scope = parent_jump;
				parent_jump->top_path[n_index]->scope_location = SCOPE_LOCATION_TOP;
				parent_jump->top_path[n_index]->scope_child_index = -1;
				parent_jump->top_path[n_index]->scope_node_index = n_index;
			}
		} else {
			// this->explore_node->scope_location == SCOPE_LOCATION_BRANCH
			for (int n_index = this->explore_node->scope_node_index+1; n_index < this->parent_jump_end_non_inclusive_index; n_index++) {
				delete parent_jump->children_nodes[explore_node->scope_child_index][n_index];
			}
			parent_jump->children_nodes[explore_node->scope_child_index].erase(
				parent_jump->children_nodes[explore_node->scope_child_index].begin() + this->explore_node->scope_node_index+1,
				parent_jump->children_nodes[explore_node->scope_child_index].begin() + this->parent_jump_end_non_inclusive_index);

			parent_jump->children_nodes[explore_node->scope_child_index].insert(
				parent_jump->children_nodes[explore_node->scope_child_index].begin(),
				this->explore_path.begin(), this->explore_path.end());
			this->explore_node->next = this->explore_path[0];
			if (this->parent_jump_end_non_inclusive_index >= (int)parent_jump->children_nodes[explore_node->scope_child_index].size()) {
				this->explore_path[this->explore_path.size()-1]->next = parent_jump;
			} else {
				this->explore_path[this->explore_path.size()-1]->next = parent_jump->children_nodes[explore_node->scope_child_index][this->parent_jump_end_non_inclusive_index];
			}

			for (int n_index = 0; n_index < (int)parent_jump->children_nodes[explore_node->scope_child_index].size(); n_index++) {
				parent_jump->children_nodes[explore_node->scope_child_index][n_index]->parent_scope = parent_jump;
				parent_jump->children_nodes[explore_node->scope_child_index][n_index]->scope_location = SCOPE_LOCATION_BRANCH;
				parent_jump->children_nodes[explore_node->scope_child_index][n_index]->scope_child_index = explore_node->scope_child_index;
				parent_jump->children_nodes[explore_node->scope_child_index][n_index]->scope_node_index = n_index;
			}
		}
	}
}

void CandidateReplace::clean() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
}
