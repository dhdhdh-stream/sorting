#ifndef SOLUTION_H
#define SOLUTION_H

#include <mutex>

#include "explore.h"
#include "solution_node.h"

class Explore;
class SolutionNode;
class Solution {
public:
	Explore* explore;

	long int id;

	std::vector<SolutionNode*> nodes;
	std::mutex nodes_mtx;
	
	int current_state_counter;
	std::mutex state_mtx;

	int current_potential_state_counter;
	std::mutex potential_state_mtx;

	double average_score;

	std::mutex display_mtx;

	Solution(Explore* explore);
	Solution(Explore* explore,
			 std::ifstream& save_file);
	~Solution();

	void iteration(bool tune,
				   bool save_for_display);

	void save(std::ofstream& save_file);
};

#endif /* SOLUTION_H */