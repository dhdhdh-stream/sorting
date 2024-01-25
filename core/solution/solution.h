/**
 * - world modeling cannot be used for innovation
 *   - even small changes can lead to unique outcomes and cannot predict what hasn't been tried
 *     - cannot even predict if something is familiar so no need to try further
 *       - as new changes may lead to better predictions
 * 
 * - world modeling is good for surviving
 *   - which, for humans, is needed to reach the frontiers of knowledge
 *     - so world modeling is a foundation for innovation for humans
 * 
 * - innovation is simply slow and costly
 *   - only way to speed things up is to cheat, i.e, to learn
 */

#ifndef SOLUTION_H
#define SOLUTION_H

#include <fstream>
#include <map>
#include <set>
#include <vector>

#include "problem.h"

class AbstractExperiment;
class AbstractNode;
class OuterExperiment;
class Scope;
class ScopeNode;
class State;

#if defined(MDEBUG) && MDEBUG
const int STARTING_NUM_DATAPOINTS = 10;
#else
const int STARTING_NUM_DATAPOINTS = 200;
#endif /* MDEBUG */

class Solution {
public:
	int id;

	int state_counter;
	std::map<int, State*> states;
	/**
	 * - use map to track states as may be sparse
	 */
	/**
	 * TODO: track relations among states, e.g.:
	 *   - which states are used together
	 *   - which states depend on which states
	 */

	int scope_counter;
	std::map<int, Scope*> scopes;

	Scope* root;

	int max_depth;	// max depth for run that concluded -> set limit to max_depth+10/1.2*max_depth
	int depth_limit;

	// TODO: track successful action counts
	// TODO: track recent updates, and add explore weight

	int curr_num_datapoints;

	OuterExperiment* outer_experiment;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	// TODO: potentially check for exceed max_depth
	std::vector<Problem*> verify_problems;
	std::vector<unsigned long> verify_seeds;
	#endif /* MDEBUG */

	Solution();
	~Solution();

	void init();
	void load(std::string path,
			  std::string name);

	void success_reset();
	void fail_reset();

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::string path,
			  std::string name);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */