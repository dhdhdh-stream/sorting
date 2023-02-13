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

	double average_score;
	Scope* root;

	std::vector<Scope*> scope_dictionary;
	// TODO: try MCTS with fading
	// TODO: the stat that is important is whether added to solution

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void new_sequence(int& sequence_length,
					  std::vector<bool>& is_inner_scope,
					  std::vector<int>& existing_scope_ids,
					  std::vector<Action>& actions,
					  bool can_be_empty);

	void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */