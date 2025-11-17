#ifndef LOGIC_TREE_H
#define LOGIC_TREE_H

#include <fstream>
#include <map>
#include <vector>

class AbstractLogicNode;

class LogicTree {
public:
	int node_counter;
	std::map<int, AbstractLogicNode*> nodes;

	AbstractLogicNode* root;

	std::vector<double> improvement_history;

	~LogicTree();

	void save(std::string path,
			  std::string name);
	void load(std::string path,
			  std::string name);
};

#endif /* LOGIC_TREE_H */