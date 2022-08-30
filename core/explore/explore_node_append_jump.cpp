#include "explore_node_append_jump.h"

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "solution_node_action.h"
#include "solution_node_if_start.h"
#include "solution_node_if_end.h"

using namespace std;

ExploreNodeAppendJump::ExploreNodeAppendJump(Explore* explore,
											 int jump_start_node_index,
											 int new_child_index,
											 vector<int> new_path_node_indexes) {
	this->explore = explore;

	this->type = EXPLORE_APPEND_JUMP;

	this->jump_start_node_index = jump_start_node_index;
	this->new_child_index = new_child_index;
	this->new_path_node_indexes = new_path_node_indexes;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeAppendJump::ExploreNodeAppendJump(Explore* explore,
											 ifstream& save_file) {
	this->explore = explore;

	this->type = EXPLORE_APPEND_JUMP;

	string jump_start_node_index_line;
	getline(save_file, jump_start_node_index_line);
	this->jump_start_node_index = stoi(jump_start_node_index_line);

	string new_child_index_line;
	getline(save_file, new_child_index_line);
	this->new_child_index = stoi(new_child_index_line);

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

ExploreNodeAppendJump::~ExploreNodeAppendJump() {
	// do nothing
}

void ExploreNodeAppendJump::process() {
	SolutionNodeIfStart* start_node = (SolutionNodeIfStart*)this->explore->solution->nodes[this->jump_start_node_index];
	SolutionNodeIfEnd* end_node = start_node->end;

	if (this->new_path_node_indexes.size() == 0) {
		start_node->children_nodes[this->new_child_index] = end_node;
	} else {
		SolutionNodeAction* curr_node = (SolutionNodeAction*)this->explore->solution->nodes[this->new_path_node_indexes[0]];
		curr_node->node_is_on = true;
		start_node->children_nodes[this->new_child_index] = curr_node;
		curr_node->previous = start_node;
		for (int p_index = 1; p_index < (int)this->new_path_node_indexes.size(); p_index++) {
			SolutionNodeAction* next_node = (SolutionNodeAction*)this->explore->solution->nodes[this->new_path_node_indexes[p_index]];
			next_node->node_is_on = true;
			curr_node->next = next_node;
			next_node->previous = curr_node;
			curr_node = next_node;
		}
		curr_node->next = end_node;
	}
	start_node->children_on[this->new_child_index] = true;
}

void ExploreNodeAppendJump::save(std::ofstream& save_file) {
	save_file << this->jump_start_node_index << endl;

	save_file << this->new_child_index << endl;

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
