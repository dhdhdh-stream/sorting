#include "explore_node_loop.h"

#include "explore_node_new_jump.h"
#include "explore_node_append_jump.h"
#include "explore_node_loop.h"
#include "explore_node_state.h"
#include "explore_utilities.h"
#include "solution_node_loop_start.h"
#include "solution_node_loop_end.h"

using namespace std;

ExploreNodeLoop::ExploreNodeLoop(Explore* explore,
								 int loop_start_non_inclusive_index,
								 int loop_start_inclusive_index,
								 int loop_end_inclusive_index,
								 int loop_end_non_inclusive_index,
								 int new_start_node_index,
								 int new_end_node_index) {
	this->explore = explore;

	this->type = EXPLORE_LOOP;

	this->loop_start_non_inclusive_index = loop_start_non_inclusive_index;
	this->loop_start_inclusive_index = loop_start_inclusive_index;
	this->loop_end_inclusive_index = loop_end_inclusive_index;
	this->loop_end_non_inclusive_index = loop_end_non_inclusive_index;

	this->new_start_node_index = new_start_node_index;
	this->new_end_node_index = new_end_node_index;

	this->count = 0;
	this->average_score = 0.0;
	this->average_misguess = 1.0;
}

ExploreNodeLoop::ExploreNodeLoop(Explore* explore,
								 ifstream& save_file) {
	this->explore = explore;

	this->type = EXPLORE_LOOP;

	string loop_start_non_inclusive_index_line;
	getline(save_file, loop_start_non_inclusive_index_line);
	this->loop_start_non_inclusive_index = stoi(loop_start_non_inclusive_index_line);

	string loop_start_inclusive_index_line;
	getline(save_file, loop_start_inclusive_index_line);
	this->loop_start_inclusive_index = stoi(loop_start_inclusive_index_line);

	string loop_end_inclusive_index_line;
	getline(save_file, loop_end_inclusive_index_line);
	this->loop_end_inclusive_index = stoi(loop_end_inclusive_index_line);

	string loop_end_non_inclusive_index_line;
	getline(save_file, loop_end_non_inclusive_index_line);
	this->loop_end_non_inclusive_index = stoi(loop_end_non_inclusive_index_line);

	string new_start_node_index_line;
	getline(save_file, new_start_node_index_line);
	this->new_start_node_index = stoi(new_start_node_index_line);

	string new_end_node_index_line;
	getline(save_file, new_end_node_index_line);
	this->new_end_node_index = stoi(new_end_node_index_line);

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

ExploreNodeLoop::~ExploreNodeLoop() {
	// do nothing
}

void ExploreNodeLoop::process() {
	SolutionNodeLoopStart* new_start_node = (SolutionNodeLoopStart*)this->explore->solution->nodes[this->new_start_node_index];
	new_start_node->node_is_on = true;
	SolutionNodeLoopEnd* new_end_node = (SolutionNodeLoopEnd*)this->explore->solution->nodes[this->new_end_node_index];
	new_end_node->node_is_on = true;

	new_start_node->end = new_end_node;
	new_end_node->start = new_start_node;

	SolutionNode* loop_start_non_inclusive = this->explore->solution->nodes[this->loop_start_non_inclusive_index];
	new_start_node->previous = loop_start_non_inclusive;
	SolutionNode* loop_end_non_inclusive = this->explore->solution->nodes[this->loop_end_non_inclusive_index];
	new_end_node->next = loop_end_non_inclusive;
	set_previous_if_needed(loop_end_non_inclusive,
						   new_end_node);

	SolutionNode* loop_start_inclusive = this->explore->solution->nodes[this->loop_start_inclusive_index];
	SolutionNode* loop_end_inclusive = this->explore->solution->nodes[this->loop_end_inclusive_index];

	set_next(loop_start_non_inclusive,
			 loop_start_inclusive,
			 new_start_node);
	new_start_node->next = loop_start_inclusive;
	set_next(loop_end_inclusive,
			 loop_end_non_inclusive,
			 new_end_node);
}

void ExploreNodeLoop::save(ofstream& save_file) {
	save_file << this->loop_start_non_inclusive_index << endl;
	save_file << this->loop_start_inclusive_index << endl;
	save_file << this->loop_end_inclusive_index << endl;
	save_file << this->loop_end_non_inclusive_index << endl;

	save_file << this->new_start_node_index << endl;
	save_file << this->new_end_node_index << endl;

	save_file << this->children.size() << endl;
	for (int c_index = 0; c_index < (int)this->children.size(); c_index++) {
		save_file << this->children[c_index]->type << endl;
		this->children[c_index]->save(save_file);
	}

	save_file << this->count << endl;
	save_file << this->average_score << endl;
	save_file << this->average_misguess << endl;
}
