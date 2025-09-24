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
#include <list>
#include <map>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

const int NUM_LAST_EXPERIMENT_TRACK = 10;
const double LAST_EXPERIMENT_BETTER_THAN_RATIO = 0.8;

class Solution {
public:
	int timestamp;
	double curr_score;

	std::map<int, Scope*> scopes;

	std::list<double> last_experiment_scores;
	/**
	 * - even if experiment safe to add, doesn't mean should
	 *   - noise leads to more noise
	 * 
	 * - but accept if no progress has been made recently
	 */

	Solution();
	~Solution();

	void load(std::ifstream& input_file);

	void clean_inputs(Scope* scope,
					  int node_id);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void clean_scopes();

	void save(std::ofstream& output_file);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */