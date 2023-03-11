#ifndef SOLUTION_H
#define SOLUTION_H

#include <vector>

#include "scope.h"

class Solution {
public:
	double average_score;
	Scope* root;	// starts with ACTION_START

	std::vector<Scope*> scopes;
	// TODO: when cleaning, instead of changing indexes, swap in blank placeholders

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	// Solution(std::ifstream& input_file);
	~Solution();

	// TODO: to find sequences, travel through solution
	// void new_sequence(int& sequence_length,
	// 				  std::vector<bool>& is_inner_scope,
	// 				  std::vector<int>& existing_scope_ids,
	// 				  std::vector<Action>& actions,
	// 				  bool can_be_empty);

	// void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */