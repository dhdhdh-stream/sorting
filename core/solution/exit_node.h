#ifndef EXIT_NODE_H
#define EXIT_NODE_H

#include <fstream>

#include "abstract_node.h"

class ExitNode : public AbstractNode {
public:
	int exit_depth;

	int exit_node_parent_id;
	int exit_node_id;
	AbstractNode* exit_node;

	ExitNode();
	~ExitNode();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

#endif /* EXIT_NODE_H */