#include "explore.h"

#include <iostream>
#include <random>

#include "definitions.h"
#include "explore_node_root.h"
#include "explore_node_new_jump.h"
#include "explore_node_loop.h"
#include "utilities.h"

using namespace std;

Explore::Explore() {
	this->root = new ExploreNodeRoot(this);

	this->solution = new Solution(this);
}

Explore::Explore(ifstream& save_file) {
	this->root = new ExploreNodeRoot(this, save_file);

	this->solution = new Solution(this, save_file);
}

Explore::~Explore() {
	delete this->root;

	delete this->solution;
}

void Explore::setup_cycle() {
	for (int n_index = 0; n_index < (int)this->solution->nodes.size(); n_index++) {
		this->solution->nodes[n_index]->reset();
	}

	ExploreNode* curr_node = this->root;
	while (true) {
		curr_node->process();

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

		curr_node = curr_node->children[best_index];
	}

	this->solution->current_potential_state_counter = 0;
}

void Explore::cleanup_cycle() {
	for (int n_index = 0; n_index < (int)this->solution->nodes.size(); n_index++) {
		if (this->solution->nodes[n_index]->node_is_on) {
			this->solution->nodes[n_index]->clear_potential_state();
			this->solution->nodes[n_index]->clear_explore();
		}
	}
}

void Explore::save() {
	ofstream save_file;
	string save_file_name = "../saves/" + to_string(time(NULL)) + ".txt";
	save_file.open(save_file_name);

	this->root->save(save_file);

	this->solution->save(save_file);

	save_file.close();
}
