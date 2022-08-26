#include "explore_node_new_jump.h"

#include <iostream>

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "explore_node_state.h"
#include "explore_utilities.h"
#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"

using namespace std;

ExploreNodeNewJump::ExploreNodeNewJump(Explore* explore,
									   int jump_start_non_inclusive_index,
									   int jump_start_inclusive_index,
									   int jump_end_inclusive_index,
									   int jump_end_non_inclusive_index,
									   int new_start_node_index,
									   int new_end_node_index,
									   vector<int> new_path_node_indexes) {
	this->explore = explore;

	this->type = EXPLORE_NEW_JUMP;

	this->jump_start_non_inclusive_index = jump_start_non_inclusive_index;
	this->jump_start_inclusive_index = jump_start_inclusive_index;
	this->jump_end_inclusive_index = jump_end_inclusive_index;
	this->jump_end_non_inclusive_index = jump_end_non_inclusive_index;

	this->new_start_node_index = new_start_node_index;
	this->new_end_node_index = new_end_node_index;

	this->new_path_node_indexes = new_path_node_indexes;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeNewJump::ExploreNodeNewJump(Explore* explore,
									   ifstream& save_file) {
	this->explore = explore;

	this->type = EXPLORE_NEW_JUMP;

	string jump_start_non_inclusive_index_line;
	getline(save_file, jump_start_non_inclusive_index_line);
	this->jump_start_non_inclusive_index = stoi(jump_start_non_inclusive_index_line);

	string jump_start_inclusive_index_line;
	getline(save_file, jump_start_inclusive_index_line);
	this->jump_start_inclusive_index = stoi(jump_start_inclusive_index_line);

	string jump_end_inclusive_index_line;
	getline(save_file, jump_end_inclusive_index_line);
	this->jump_end_inclusive_index = stoi(jump_end_inclusive_index_line);

	string jump_end_non_inclusive_index_line;
	getline(save_file, jump_end_non_inclusive_index_line);
	this->jump_end_non_inclusive_index = stoi(jump_end_non_inclusive_index_line);

	string new_start_node_index_line;
	getline(save_file, new_start_node_index_line);
	this->new_start_node_index = stoi(new_start_node_index_line);

	string new_end_node_index_line;
	getline(save_file, new_end_node_index_line);
	this->new_end_node_index = stoi(new_end_node_index_line);

	string new_path_num_nodes_line;
	getline(save_file, new_path_num_nodes_line);
	int new_path_num_nodes = stoi(new_path_num_nodes_line);
	for (int n_index = 0; n_index < new_path_num_nodes; n_index++) {
		string node_index_line;
		getline(save_file, node_index_line);
		this->new_path_node_indexes.push_back(stoi(node_index_line));
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

ExploreNodeNewJump::~ExploreNodeNewJump() {
	// do nothing
}

void ExploreNodeNewJump::process() {
	SolutionNodeIfStart* new_start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->new_start_node_index];
	new_start_node->node_is_on = true;
	SolutionNodeIfEnd* new_end_node = (SolutionNodeIfEnd*)this->explore->solution->nodes[this->new_end_node_index];
	new_end_node->node_is_on = true;

	new_start_node->end = new_end_node;
	new_end_node->start = new_start_node;

	SolutionNode* jump_start_non_inclusive = this->explore->solution->nodes[this->jump_start_non_inclusive_index];
	new_start_node->previous = jump_start_non_inclusive;
	SolutionNode* jump_end_non_inclusive = this->explore->solution->nodes[this->jump_end_non_inclusive_index];
	new_end_node->next = jump_end_non_inclusive;
	set_previous_if_needed(jump_end_non_inclusive,
						   new_end_node);

	if (jump_start_inclusive_index == -1) {	// && jump_end_inclusive_index == -1
		set_next(jump_start_non_inclusive,
				 jump_end_non_inclusive,
				 new_start_node);
		new_start_node->children_nodes[0] = new_end_node;
	} else {
		SolutionNode* jump_start_inclusive = this->explore->solution->nodes[this->jump_start_inclusive_index];
		SolutionNode* jump_end_inclusive = this->explore->solution->nodes[this->jump_end_inclusive_index];

		set_next(jump_start_non_inclusive,
				 jump_start_inclusive,
				 new_start_node);
		new_start_node->children_nodes[0] = jump_start_inclusive;
		set_previous_if_needed(jump_start_inclusive,
							   new_start_node);
		set_next(jump_end_inclusive,
				 jump_end_non_inclusive,
				 new_end_node);
	}
	new_start_node->children_on[0] = true;

	if (this->new_path_node_indexes.size() == 0) {
		new_start_node->children_nodes[1] = new_end_node;
	} else {
		SolutionNodeAction* curr_node = (SolutionNodeAction*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
		curr_node->node_is_on = true;
		new_start_node->children_nodes[1] = curr_node;
		curr_node->previous = new_start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNodeAction* next_node = (SolutionNodeAction*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
			next_node->node_is_on = true;
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = new_end_node;
	}
	new_start_node->children_on[1] = true;
}

void ExploreNodeNewJump::save(ofstream& save_file) {
	save_file << this->jump_start_non_inclusive_index << endl;
	save_file << this->jump_start_inclusive_index << endl;
	save_file << this->jump_end_inclusive_index << endl;
	save_file << this->jump_end_non_inclusive_index << endl;

	save_file << this->new_start_node_index << endl;
	save_file << this->new_end_node_index << endl;

	save_file << this->new_path_node_indexes.size() << endl;
	for (int n_index = 0; n_index < (int)this->new_path_node_indexes.size(); n_index++) {
		save_file << this->new_path_node_indexes[n_index] << endl;
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
