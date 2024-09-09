#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class Problem;
class Solution;

const int INPUT_TYPE_GLOBAL = 0;
const int INPUT_TYPE_RELATIVE = 1;
const int INPUT_TYPE_HISTORY = 2;

class BranchNode : public AbstractNode {
public:
	std::vector<int> input_types;
	std::vector<vector<double>> input_locations;
	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;
	Network* network;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

};

#endif /* BRANCH_NODE_H */