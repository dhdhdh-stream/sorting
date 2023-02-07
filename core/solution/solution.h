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

	std::vector<Action> action_dictionary;
	std::vector<int> action_last_success;

	std::vector<Scope*> scope_dictionary;
	std::vector<int> scope_last_success;

	int new_sequence_index;

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void new_sequence(int& sequence_length,
					  std::vector<int>& new_sequence_types,
					  std::vector<int>& existing_scope_ids,
					  std::vector<int>& existing_action_ids,
					  std::vector<Action>& new_actions,
					  bool can_be_empty);
	void new_sequence_success(int sequence_length,
							  std::vector<int>& new_sequence_types,
							  std::vector<int>& existing_scope_ids,
							  std::vector<int>& existing_action_ids,
							  std::vector<Action>& new_actions);
	void new_sequence_iter();

	void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */