#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <mutex>
#include <vector>

#include "scope.h"

class Solution {
public:
	int id_counter;
	std::mutex id_counter_mtx;

	Scope* root;

	// use refs, allow duplicates, choose randomly, so successful actions are naturally chosen more
	// on reload, start from a copy of everything currently in the solution
	std::vector<Action> action_dictionary;
	std::vector<Scope*> scope_dictionary;
	// TODO: deep copy scopes for scope_dictionary refs as original might get deleted?

	Solution();
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	void new_sequence(int& sequence_length,
					  std::vector<bool>& is_existing,
					  std::vector<Scope*>& existing_actions,
					  std::vector<Action>& actions);

	void save(std::ofstream& output_file);
};

#endif /* SOLUTION_H */