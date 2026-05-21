#ifndef SOLUTION_H
#define SOLUTION_H

#include <map>
#include <vector>

class AbstractNode;

class Solution {
public:
	int node_counter;
	std::map<int, AbstractNode*> nodes;



	void random_exit_activate(AbstractNode* starting_node,
							  std::vector<AbstractNode*>& possible_exits);

};

#endif /* SOLUTION_H */