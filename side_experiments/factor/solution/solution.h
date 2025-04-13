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

class AbstractNode;
class Problem;
class Scope;

const int INCREASE_TIME_PENALTY_ITER = 6;
const int DECREASE_TIME_PENALTY_ITER = 2;

class Solution {
public:
	int timestamp;
	double curr_score;

	double curr_true_score;
	double best_true_score;
	int best_true_score_timestamp;
	/**
	 * TODO: use actual time spent?
	 */
	double curr_time_penalty;

	std::vector<Scope*> scopes;

	std::vector<double> obs_average_vals;
	std::vector<double> obs_variances;

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

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_scopes();

	void clean();
	void measure_update();

	void save(std::string path,
			  std::string name);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */