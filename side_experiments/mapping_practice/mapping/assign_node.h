#ifndef ASSIGN_NODE_H
#define ASSIGN_NODE_H

#include <utility>
#include <vector>

class AssignNode {
public:
	bool is_fail;

	std::vector<std::pair<int,int>> move;
	std::vector<AssignNode*> children;

	bool solve(std::vector<int>& obs,
			   std::vector<int>& actions,
			   int curr_index,
			   int curr_x,
			   int curr_y,
			   std::vector<std::vector<int>>& map_vals,
			   std::vector<std::vector<bool>>& map_assigned);
};

#endif /* ASSIGN_NODE_H */