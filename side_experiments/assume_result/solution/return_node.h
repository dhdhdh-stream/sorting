/**
 * - very useful?
 *   - makes exploring deeper much easier where there is return node after
 */

// TODO: try adding return branch nodes as well

#ifndef RETURN_NODE_H
#define RETURN_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class Problem;

class ReturnNode : public AbstractNode {
public:
	int previous_location_id;
	AbstractNode* previous_location;

	int next_node_id;
	AbstractNode* next_node;

	ReturnNode();
	ReturnNode(ReturnNode* original);
	~ReturnNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void explore_activate(Problem* problem,
						  std::vector<ContextLayer>& context);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

#endif /* RETURN_NODE_H */