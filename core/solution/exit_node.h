#ifndef EXIT_NODE_H
#define EXIT_NODE_H

#include <fstream>

#include "abstract_node.h"

class ExitNode : public AbstractNode {
public:
	int exit_depth;
	int exit_node_id;

	ExitNode();
	ExitNode(std::ifstream& input_file,
			 int id);
	~ExitNode();

	void save(std::ofstream& output_file);
	void save_for_display(std::ofstream& output_file);
};

class ExitNodeHistory : public AbstractNodeHistory {
public:
	ExitNodeHistory(ExitNode* node);
};

#endif /* EXIT_NODE_H */