/**
 * - real problems require certain things done in certain places, and
 *   certain things done in other places, but especially, they require
 *   lots of things done in lots of places
 * 
 * - so the way solutions are built is:
 *   - initially, find things that are good to do and places to repeat
 *   - after hitting local optima:
 *     - find new things that are good to do near current boundaries
 *     - repeat for more coverage, but at the cost of duplicate effort
 *       - then while removing duplicate, improve shape of what's done
 *   - improved shape enables deeper repetition, and so on
 *   - eventually, solution will cover the different things that need
 *     to be done everywhere
 * 
 * - do not remember everything tried but not taken
 *   - information becomes irrelevant as solution changes
 *     - TODO: though can prevent wasted effort in the short run
 * 
 * - cannot align different solutions/samples
 *   - built on different assumptions/choices throughout
 *   - don't create "average"/high variance solution either
 *     - makes progress harder
 *       - even when variance needed, better to start from hard connections
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class AbstractNode;
class Problem;
class Scope;

class Solution {
public:
	int timestamp;
	double average_score;

	std::vector<Scope*> scopes;

	int max_num_actions;
	int num_actions_limit;

	double existing_average_score;
	/**
	 * - add to scopes, and remove after
	 */
	Scope* subproblem;
	AbstractNode* subproblem_starting_node;
	bool subproblem_is_branch;
	AbstractNode* subproblem_exit_node;
	/**
	 * - for creating subproblem
	 */
	AbstractNode* last_experiment_node;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	Solution(Solution* original);
	~Solution();

	void init();
	void load(std::string path,
			  std::string name);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void clean();

	void create_subproblem();
	void merge_subproblem();

	void save(std::string path,
			  std::string name);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */