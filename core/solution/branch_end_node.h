/**
 * - having branch ends/more restrictive structure makes solutions significantly easier to describe
 *   - so may better match how humans work/what is correct?
 */

#ifndef BRANCH_END_NODE_H
#define BRANCH_END_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class BranchEndNode : public AbstractNode {
public:
	int branch_start_node_id;
	AbstractNode* branch_start_node;

	int next_node_id;
	AbstractNode* next_node;

	BranchEndNode();
	BranchEndNode(BranchEndNode* original);
	~BranchEndNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  std::map<AbstractNode*, AbstractNodeHistory*>& node_histories);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

#endif /* BRANCH_END_NODE_H */