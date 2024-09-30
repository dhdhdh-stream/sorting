#ifndef MARKOV_NODE_H
#define MARKOV_NODE_H

#include "abstract_node.h"
#include "action.h"
#include "context_layer.h"
#include "run_helper.h"

#include <fstream>
#include <vector>

class Network;
class Problem;
class Solution;

const int MARKOV_NODE_MAX_ITERS = 10;

const int MARKOV_NODE_ANALYZE_SIZE = 2;		// temp

const int MARKOV_STEP_TYPE_ACTION = 0;
const int MARKOV_STEP_TYPE_SCOPE = 1;

class MarkovNode : public AbstractNode {
public:
	std::vector<std::vector<int>> step_types;
	std::vector<std::vector<Action>> actions;
	std::vector<std::vector<Scope*>> scopes;
	std::vector<Network*> networks;
	Network* halt_network;

	int next_node_id;
	AbstractNode* next_node;

	MarkovNode();
	MarkovNode(MarkovNode* original,
			   Solution* parent_solution);
	~MarkovNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

#endif /* MARKOV_NODE_H */