#include "decision_tree.h"

#include <iostream>

#include "constants.h"
#include "eval_node.h"
#include "globals.h"
#include "network.h"

using namespace std;

void DecisionTree::init_helper() {
	Network* eval_network = new Network(this->obs_histories[0].size());
	uniform_int_distribution<int> distribution(0, this->obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = distribution(generator);

		eval_network->activate(this->obs_histories[index]);

		double error = this->target_val_histories[index] - eval_network->output->acti_vals[0];

		eval_network->backprop(error);
	}

	EvalNode* new_eval_node = new EvalNode();
	new_eval_node->id = this->node_counter;
	this->node_counter++;
	this->nodes[new_eval_node->id] = new_eval_node;

	new_eval_node->network = eval_network;

	this->root = new_eval_node;

	measure_helper();
}
