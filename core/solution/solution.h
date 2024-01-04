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
class State;

#if defined(MDEBUG) && MDEBUG
const int STARTING_NUM_DATAPOINTS = 20;
#else
const int STARTING_NUM_DATAPOINTS = 2000;
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
	void load(std::string name);

	void success_reset();
	void fail_reset();

	#if defined(MDEBUG) && MDEBUG
	void clear_verify();
	#endif /* MDEBUG */

	void save(std::string name);
	void save_for_display(std::ofstream& output_file);
};

#endif /* SOLUTION_H */