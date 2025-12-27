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
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class Problem;
class ProblemType;
class Scope;
class ScopeHistory;
class Tunnel;

class Solution {
public:
	/**
	 * - -1 if done
	 */
	int timestamp;
	double curr_score;

	std::vector<Scope*> scopes;

	std::vector<std::vector<double>> obs_histories;
	std::vector<double> target_val_histories;

	std::vector<double> improvement_history;
	std::vector<std::string> change_history;

	std::vector<Tunnel*> tunnel_history;

	#if defined(MDEBUG) && MDEBUG
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	Solution(Solution* original);
	~Solution();

	void init(ProblemType* problem_type);
	void load(std::ifstream& input_file);

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void clean_scopes();

	void save(std::ofstream& output_file);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */