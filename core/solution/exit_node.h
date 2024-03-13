#ifndef EXIT_NODE_H
#define EXIT_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "context_layer.h"
#include "run_helper.h"

class Scope;

class ExitNode : public AbstractNode {
public:
	std::vector<int> scope_context_ids;
	std::vector<Scope*> scope_context;
	std::vector<int> node_context_ids;
	std::vector<AbstractNode*> node_context;
	int exit_depth;

	int throw_id;

	ExitNode();
	~ExitNode();

	void activate(std::vector<ContextLayer>& context,
				  int& exit_depth,
				  RunHelper& run_helper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

#endif /* EXIT_NODE_H */