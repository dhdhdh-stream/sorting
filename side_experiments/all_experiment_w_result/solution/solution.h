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
#include <utility>
#include <vector>

class AbstractExperiment;
class AbstractNode;
class Problem;
class Scope;
class ScopeHistory;

const int SOLUTION_STATE_NON_OUTER = 0;
const int SOLUTION_STATE_OUTER = 1;

class Solution {
public:
	int timestamp;
	double curr_score;

	int state;

	std::vector<Scope*> scopes;

	std::vector<Scope*> outer_scopes;
	std::vector<int> outer_root_scope_ids;

	std::vector<double> score_histories;
	std::vector<int> num_experiment_histories;
	int score_index;

	/**
	 * - even if experiment safe to add, doesn't mean should
	 *   - noise leads to more noise
	 * 
	 * - but accept if no progress has been made recently
	 * 
	 * - don't track global
	 *   - as solution becomes fractured, becomes dominated by noise
	 *   - if experiment has good local, but poor global, will add anyways
	 */
	std::list<double> last_scores;

	std::vector<double> improvement_history;
	std::vector<std::string> change_history;

	Solution();
	~Solution();

	void load(std::ifstream& input_file);

	void clean_scopes();

	void merge_outer();

	void save(std::ofstream& output_file);

	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */