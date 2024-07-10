#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class Problem;
class Scope;

class Solution {
public:
	int generation;

	int last_updated_scope_id;
	int last_new_scope_id;

	std::vector<Scope*> scopes;

	int max_num_actions;
	int num_actions_limit;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	Solution(Solution* original);
	~Solution();

	void init();
	void load(std::ifstream& input_file);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void merge_and_delete(Solution* original_solution);

	void save(std::ofstream& output_file);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */