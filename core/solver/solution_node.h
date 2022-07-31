#ifndef SOLUTION_NODE_H
#define SOLUTION_NODE_H

#include <fstream>
#include <vector>

#include "action.h"
#include "network.h"

class SolutionNode {
public:
	int node_index;

	std::vector<int> children_indexes;
	std::vector<Action> children_actions;
	std::vector<Network*> children_networks;
	std::vector<std::string> children_network_names;

	int count;
	double information;

	SolutionNode(int node_index);
	SolutionNode(int node_index,
				 std::ifstream& save_file);
	~SolutionNode();

	void save(std::ofstream& save_file);
};

#endif /* SOLUTION_NODE_H */