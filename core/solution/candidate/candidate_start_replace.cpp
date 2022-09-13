#include "candidate_start_replace.h"

#include "definitions.h"

using namespace std;

CandidateStartReplace::CandidateStartReplace(StartScope* start_scope,
											 int replace_type,
											 double score_increase,
											 double info_gain,
											 int jump_end_non_inclusive_index,
											 std::vector<SolutionNode*> explore_path) {
	this->type = CANDIDATE_START_REPLACE;

	this->start_scope = start_scope;
	this->replace_type = replace_type;
	this->score_increase = score_increase;
	this->info_gain = info_gain;
	this->jump_end_non_inclusive_index = jump_end_non_inclusive_index;
	this->explore_path = explore_path;
}

void CandidateStartReplace::apply() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		this->explore_path[n_index]->set_is_temp_node(false);
	}

	if (this->explore_path.size() > 1
			|| this->explore_path[0]->node_type != NODE_TYPE_EMPTY) {
		vector<SolutionNode*> explore_path_deep_copy;
		for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
			explore_path_deep_copy.push_back(this->explore_path[n_index]->deep_copy(1));
		}
		action_dictionary->actions.push_back(explore_path_deep_copy);
	}

	if (this->jump_end_non_inclusive_index >= (int)this->start_scope->path.size()) {
		this->explore_path[this->explore_path.size()-1]->next = this->start_scope;
	} else {
		this->explore_path[this->explore_path.size()-1]->next = this->start_scope->path[this->jump_end_non_inclusive_index];
	}
	for (int n_index = 0; n_index < this->jump_end_non_inclusive_index; n_index++) {
		delete this->start_scope->path[n_index];
	}
	this->start_scope->path.erase(this->start_scope->path.begin(),
		this->start_scope->path.begin() + jump_end_non_inclusive_index);
	this->start_scope->path.insert(this->start_scope->path.begin(),
		this->explore_path.begin(), this->explore_path.end());

	for (int n_index = 0; n_index < (int)this->start_scope->path.size(); n_index++) {
		this->start_scope->path[n_index]->parent_scope = this->start_scope;
		this->start_scope->path[n_index]->scope_location = -1;
		this->start_scope->path[n_index]->scope_child_index = -1;
		this->start_scope->path[n_index]->scope_node_index = n_index;
	}
}

void CandidateStartReplace::clean() {
	for (int n_index = 0; n_index < (int)this->explore_path.size(); n_index++) {
		delete this->explore_path[n_index];
	}
}
