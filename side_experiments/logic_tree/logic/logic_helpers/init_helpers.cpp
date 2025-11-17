#include "logic_helpers.h"

#include "abstract_problem.h"
#include "constants.h"
#include "eval_node.h"
#include "globals.h"
#include "logic_tree.h"
#include "network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INIT_NUM_GATHER = 40;
#else
const int INIT_NUM_GATHER = 4000;
#endif /* MDEBUG */

LogicTree* init_helper(AbstractProblem* problem) {
	vector<vector<double>> obs_histories;
	vector<double> target_val_histories;
	for (int i_index = 0; i_index < INIT_NUM_GATHER; i_index++) {
		vector<double> obs;
		double target_val;
		problem->get_instance(obs,
							  target_val);

		obs_histories.push_back(obs);
		target_val_histories.push_back(target_val);
	}

	LogicTree* new_logic_tree = new LogicTree();
	new_logic_tree->node_counter = 0;

	EvalNode* new_eval_node = new EvalNode();
	new_eval_node->id = new_logic_tree->node_counter;
	new_logic_tree->node_counter++;
	new_logic_tree->nodes[new_eval_node->id] = new_eval_node;

	new_eval_node->network = new Network(obs_histories[0].size(),
										 NETWORK_SIZE_SMALL);

	uniform_int_distribution<int> distribution(0, obs_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int index = distribution(generator);

		new_eval_node->network->activate(obs_histories[index]);

		double error = target_val_histories[index] - new_eval_node->network->output->acti_vals[0];

		new_eval_node->network->backprop(error);
	}

	new_eval_node->weight = 1.0;

	new_logic_tree->root = new_eval_node;

	return new_logic_tree;
}
