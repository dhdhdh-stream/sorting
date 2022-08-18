#include "explore.h"

#include <iostream>
#include <random>

#include "definitions.h"
#include "utilities.h"

using namespace std;

Explore::Explore() {
	this->root = new ExploreRootNode();

	this->solution = new Solution();

	this->iter_index = 0;
}

Explore::~Explore() {
	delete this->root;

	delete this->solution;
}

void Explore::setup_cycle() {
	ExploreNode* curr_node = this->root;
	while (true) {
		curr_node->process();

		// TODO: revisit nodes occasionally
		if (curr_node->children.size() == 0) {
			break;
		}

		double best_uct = numeric_limits<double>::lowest();
		int best_index = -1;
		for (int c_index = 0; c_index < (int)curr_node->children.size(); c_index++) {
			double uct = curr_node->children[c_index] + \
				1.414*sqrt(log(curr_node->count+1)/(curr_node->children[c_index]->count+1));

			if (uct > best_uct) {
				best_uct = uct;
				best_index = c_index;
			}
		}
	}

	this->iter_index = 0;
}

void Explore::iteration() {
	// TODO: spend first cycles tuning
	this->solution->iteration();
}
