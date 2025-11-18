// - split conditions could be a lot more complex
//   - equals
//   - relative
//   - relative equals
//   - relative between multiple
//   - algorithmic

// - how to find?
//   - thing is, you can easily find all kinds of relations
//     - vast majority of which are not useful

// - BTW, for scopes, context can be useful to pass in as an input
//   - number of actions
//   - number of times scope has already appeared

#ifndef SPLIT_NODE_H
#define SPLIT_NODE_H

#include <fstream>

#include "abstract_logic_node.h"

class AbstractLogicNode;
class LogicTree;

class SplitNode : public AbstractLogicNode {
public:
	int obs_index;
	int split_type;
	double split_target;

	int original_node_id;
	AbstractLogicNode* original_node;
	int branch_node_id;
	AbstractLogicNode* branch_node;

	SplitNode();
	~SplitNode();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(LogicTree* logic_tree);
};

#endif /* SPLIT_NODE_H */