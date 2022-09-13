#ifndef SOLUTION_H
#define SOLUTION_H

#include <mutex>

#include "candidate.h"
#include "start_scope.h"

class Solution {
public:
	long int id;

	StartScope* start_scope;

	double average_score;

	std::vector<Candidate*> candidates;
	int explore_iters;

	std::mutex display_mtx;

	std::vector<std::vector<double>>* heap_state_vals;
	std::vector<SolutionNode*>* heap_scopes;
	std::vector<int>* heap_scope_states;
	std::vector<int>* heap_scope_locations;
	std::vector<AbstractNetworkHistory*>* heap_network_historys;
	std::vector<std::vector<double>>* heap_state_errors;

	Solution();
	Solution(std::ifstream& save_file);
	~Solution();

	void iteration(bool tune,
				   bool save_for_display);

	void save(std::ofstream& save_file);
};

#endif /* SOLUTION_H */