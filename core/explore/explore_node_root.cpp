#include "explore_node_root.h"

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "solution_node_start.h"
#include "solution_node_end.h"

using namespace std;

ExploreNodeRoot::ExploreNodeRoot(Explore* explore) {
	this->explore = explore;

	this->type = EXPLORE_ROOT;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeRoot::ExploreNodeRoot(Explore* explore,
								 std::ifstream& save_file) {
	this->explore = explore;

	this->type = EXPLORE_ROOT;

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

ExploreNodeRoot::~ExploreNodeRoot() {
	// do nothing
}

void ExploreNodeRoot::process() {
	SolutionNodeStart* start_node = (SolutionNodeStart*)this->explore->solution->nodes[0];
	start_node->node_is_on = true;
	SolutionNodeEnd* end_node = (SolutionNodeEnd*)this->explore->solution->nodes[1];
	end_node->node_is_on = true;

	start_node->next = end_node;
}

void ExploreNodeRoot::save(ofstream& save_file) {
	save_file << this->children.size() << endl;
	for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
		save_file << this->children[c_index]->type << endl;
		this->children[c_index]->save(save_file);
	}

	save_file << this->count << endl;
	save_file << this->average_score << endl;
	save_file << this->average_misguess << endl;
}
