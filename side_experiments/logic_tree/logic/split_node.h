/**
 * - don't rely on for algorithmic
 *   - let solution handle that
 * 
 * - so simply looking for splits that drive statistical learning
 */

#ifndef SPLIT_NODE_H
#define SPLIT_NODE_H

#include <fstream>

#include "abstract_logic_node.h"

class AbstractLogicNode;
class LogicTree;

class SplitNode : public AbstractLogicNode {
public:
	int obs_index;
	int rel_obs_index;
	int split_type;
	double split_target;
	double split_range;

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