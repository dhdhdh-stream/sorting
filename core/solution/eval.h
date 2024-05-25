/**
 * - target_val is made up of that which is predictable and that which isn't
 *   - by using eval, will remove impact of that which isn't predictable
 *     - reducing variance, improving training
 *   - but wil lose that which isn't yet predictable by eval
 *     - making it difficult to find sequences that lead to positive but new/strange situations
 * 
 * - run eval both at start and at end
 *   - to account for different possible starting points
 *     - greatly reducing variance
 * 
 * - eval is if completely stopped
 *   - instead of if continue as normal which negates/destroys factors
 *     - i.e., solution does something good, with a good signal - but not if compared to itself
 */

#ifndef EVAL_H
#define EVAL_H

#include <fstream>
#include <vector>

#include "run_helper.h"

class AbstractExperiment;
class AbstractNode;
class Network;
class Problem;
class Scope;
class Solution;

class EvalHistory;
class Eval {
public:
	Scope* parent_scope;

	/**
	 * - contains nodes for:
	 *   - start orientation
	 *     - starting at 0
	 *   - start eval
	 *     - starting at 1
	 *   - end orientation
	 *     - starting at 2
	 *   - end eval
	 *     - starting at 3
	 * 
	 * - new scopes can be created on orientation
	 *   - but they will have ids and their own evals and be handled normally
	 */
	Scope* subscope;

	double average_score;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<int> linear_input_indexes;
	std::vector<double> linear_weights;

	std::vector<std::vector<int>> network_input_indexes;
	Network* network;

	Eval(Scope* parent_scope);
	Eval(Eval* original,
		 Solution* parent_solution);
	~Eval();

	void activate_start(Problem* problem,
						RunHelper& run_helper,
						EvalHistory* history);
	void activate_end(Problem* problem,
					  RunHelper& run_helper,
					  EvalHistory* history);
	double calc_impact(EvalHistory* history);

	void init();
	void load(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

class EvalHistory {
public:
	Eval* eval;

	ScopeHistory* scope_history;
	int start_eval_index;
	int end_orientation_index;
	int end_eval_index;

	EvalHistory(Eval* eval);
	EvalHistory(EvalHistory* original);
	~EvalHistory();
};

#endif /* EVAL_H */