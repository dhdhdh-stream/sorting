#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <mutex>
#include <vector>

#include "scope.h"

class Solution {
public:
	int id_counter;	// for Branch, BranchPath, Fold, but not for scopes
	std::mutex id_counter_mtx;

	Scope* root;

	// allow duplicates, choose randomly, so successful actions are naturally chosen more
	// on reload, start from a copy of everything currently in the solution
	std::vector<Action> action_dictionary;

	std::vector<Scope*> scope_dictionary;
	std::vector<int> scope_use_counts;
	int scope_use_sum_count;

	// TODO: use percentages or MCTS for dictionaries

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void new_sequence(int& sequence_length,
					  std::vector<bool>& is_existing,
					  std::vector<Scope*>& existing_actions,
					  std::vector<Action>& actions,
					  bool can_be_empty);

	void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */