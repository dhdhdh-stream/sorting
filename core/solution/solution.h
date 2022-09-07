#ifndef SOLUTION_H
#define SOLUTION_H

#include <mutex>

#include "explore.h"
#include "solution_node.h"

class SolutionNode;
class Solution {
public:
	long int id;

	StartScope* start_scope;
	
	double average_score;

	std::vector<Candidate*> candidate;

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