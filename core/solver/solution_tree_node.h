#ifndef SOLUTION_TREE_NODE_H
#define SOLUTION_TREE_NODE_H

#include <fstream>
#include <vector>

#include "action.h"
#include "network.h"

class SolutionTreeNode {
public:
	SolutionTreeNode* parent;

	std::vector<SolutionTreeNode*> children;
	std::vector<Action> children_actions;
	std::vector<Network*> children_networks;
	std::vector<std::string> children_network_names;

	bool has_halt;
	Network* halt_network;
	std::string halt_network_name;

	int count;
	double score;
	double information;

	SolutionTreeNode(SolutionTreeNode* parent,
					 int count,
					 double score,
					 double information);
	SolutionTreeNode(SolutionTreeNode* parent,
					 std::ifstream& save_file);
	~SolutionTreeNode();

	void save(std::ofstream& save_file);
};

#endif /* SOLUTION_TREE_NODE_H */