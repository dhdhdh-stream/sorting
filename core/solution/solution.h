#ifndef SOLUTION_H
#define SOLUTION_H

#include <mutex>

#include "candidate.h"
#include "start_scope.h"

class Solution {
public:
	StartScope* start_scope;

	double average_score;

	std::vector<Candidate*> candidates;
	int explore_iters;

	std::mutex display_mtx;

	Solution();
	Solution(std::ifstream& save_file);
	~Solution();

	void iteration(bool tune,
				   bool save_for_display);

	void save(std::ofstream& save_file);
};

#endif /* SOLUTION_H */