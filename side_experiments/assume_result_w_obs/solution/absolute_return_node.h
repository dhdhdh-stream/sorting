#ifndef ABSOLUTE_RETURN_NODE_H
#define ABSOLUTE_RETURN_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class Problem;

class AbsoluteReturnNode : public AbstractNode {
public:
	std::vector<int> location;

	int next_node_id;
	AbstractNode* next_node;

	AbsoluteReturnNode();
	AbsoluteReturnNode(AbsoluteReturnNode* original);
	~AbsoluteReturnNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper);

	void explore_activate(Problem* problem,
						  std::vector<ContextLayer>& context,
						  RunHelper& run_helper);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 std::vector<ContextLayer>& context,
						 RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

#endif /* ABSOLUTE_RETURN_NODE_H */