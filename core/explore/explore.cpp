#include "explore.h"

#include <iostream>
#include <random>

#include "definitions.h"
#include "utilities.h"

using namespace std;

Explore::Explore() {
	this->root = new ExploreRootNode(this);

	this->solution = new Solution();
}

Explore::~Explore() {
	delete this->root;

	delete this->solution;
}

void Explore::setup_cycle() {
	for (int n_index = 0; n_index < (int)this->solution->nodes.size(); n_index++) {
		this->solution->nodes[n_index]->reset();
	}

	vector<SolutionNode*> scopes;

	ExploreNode* curr_node = this->root;
	scopes.push_back(this->solution->nodes[0]);
	while (true) {
		curr_node->process();

		if (curr_node->type == EXPLORE_NEW_JUMP) {
			ExploreNodeNewJump* curr_node_new_jump = (ExploreNodeNewJump*)curr_node;
			int scope_index = curr_node_new_jump->new_start_node_index;
			scopes.push_back(this->solution->nodes[scope_index]);
		} else if (curr_node->type == EXPLORE_LOOP) {
			ExploreNodeLoop* curr_node_loop = (ExploreNodeLoop*)curr_node;
			int scope_index = curr_node_loop->new_start_node_index;
			scopes.push_back(this->solution->nodes[scope_index]);
		}

		// TODO: revisit nodes occasionally
		if (curr_node->children.size() == 0) {
			this->current_node = curr_node;
			break;
		}

		double best_uct = numeric_limits<double>::lowest();
		int best_index = -1;
		for (int c_index = 0; c_index < (int)curr_node->children.size(); c_index++) {
			double uct = curr_node->children[c_index]->average_score + \
				1.414*sqrt(log(curr_node->count+1)/(curr_node->children[c_index]->count+1));

			if (uct > best_uct) {
				best_uct = uct;
				best_index = c_index;
			}
		}
	}

	this->solution->current_potential_state_counter = 0;
	for (int s_index = 0; s_index < (int)scopes.size(); s_index++) {
		vector<int> potential_state_indexes;
		for (int i = 0; i < 5; i++) {
			potential_state_indexes.push_back(this->solution->current_potential_state_counter);
			this->solution->current_potential_state_counter++;
		}
		scopes[s_index]->add_potential_state(potential_state_indexes,
											 scopes[s_index]);
	}
}
