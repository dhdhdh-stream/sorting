#include "explore_node_state.h"

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "explore_node_state.h"
#include "solution_node_if_start.h"
#include "solution_node_loop_start.h"

using namespace std;

ExploreNodeState::ExploreNodeState(Explore* explore,
								   int scope_index,
								   vector<int> new_state_indexes) {
	this->explore = explore;

	this->type = EXPLORE_STATE;

	this->scope_index = scope_index;
	this->new_state_indexes = new_state_indexes;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeState::ExploreNodeState(Explore* explore,
								   ifstream& save_file) {
	this->explore = explore;

	this->type = EXPLORE_STATE;

	string scope_index_line;
	getline(save_file, scope_index_line);
	this->scope_index = stoi(scope_index_line);

	string num_new_state_indexes_line;
	getline(save_file, num_new_state_indexes_line);
	int num_new_state_indexes = stoi(num_new_state_indexes_line);
	for (int s_index = 0; s_index < num_new_state_indexes; s_index++) {
		string new_state_index_line;
		getline(save_file, new_state_index_line);
		this->new_state_indexes.push_back(stoi(new_state_index_line));
	}

	string num_children_line;
	getline(save_file, num_children_line);
	int num_children = stoi(num_children_line);
	for (int c_index = 0; c_index < num_children; c_index++) {
		string node_type_line;
		getline(save_file, node_type_line);
		int node_type = stoi(node_type_line);
		ExploreNode* child_node;
		if (node_type == EXPLORE_NEW_JUMP) {
			child_node = new ExploreNodeNewJump(this->explore,
												save_file);
		} else if (node_type == EXPLORE_APPEND_JUMP) {
			child_node = new ExploreNodeAppendJump(this->explore,
												   save_file);
		} else if (node_type == EXPLORE_LOOP) {
			child_node = new ExploreNodeLoop(this->explore,
											 save_file);
		} else if (node_type == EXPLORE_STATE) {
			child_node = new ExploreNodeState(this->explore,
											  save_file);
		}
		this->children.push_back(child_node);
	}

	string count_line;
	getline(save_file, count_line);
	this->count = stoi(count_line);

	string average_score_line;
	getline(save_file, average_score_line);
	this->average_score = stof(average_score_line);

	string average_misguess_line;
	getline(save_file, average_misguess_line);
	this->average_misguess = stof(average_misguess_line);
}

ExploreNodeState::~ExploreNodeState() {
	// do nothing
}

void ExploreNodeState::process() {
	SolutionNode* scope = this->explore->solution->nodes[this->scope_index];

	if (scope->node_type == NODE_TYPE_IF_START) {
		SolutionNodeIfStart* scope_if_start = (SolutionNodeIfStart*)scope;
		scope_if_start->scope_states_on.insert(scope_if_start->scope_states_on.end(),
			this->new_state_indexes.begin(), this->new_state_indexes.end());
	} else if (scope->node_type == NODE_TYPE_LOOP_START) {
		SolutionNodeLoopStart* scope_loop_start = (SolutionNodeLoopStart*)scope;
		scope_loop_start->scope_states_on.insert(scope_loop_start->scope_states_on.end(),
			this->new_state_indexes.begin(), this->new_state_indexes.end());
	}
}

void ExploreNodeState::save(ofstream& save_file) {
	save_file << this->scope_index << endl;

	save_file << this->new_state_indexes.size() << endl;
	for (int s_index = 0; s_index < (int)this->new_state_indexes.size(); s_index++) {
		save_file << this->new_state_indexes[s_index] << endl;
	}

	save_file << this->children.size() << endl;
	for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
		save_file << this->children[c_index]->type << endl;
		this->children[c_index]->save(save_file);
	}

	save_file << this->count << endl;
	save_file << this->average_score << endl;
	save_file << this->average_misguess << endl;
}
