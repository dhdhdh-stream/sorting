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

	Scope* subscope;

	/**
	 * - applies for both start and end (and between)
	 */
	double score_average_score;

	std::vector<AbstractNode*> score_input_node_contexts;
	std::vector<int> score_input_obs_indexes;

	std::vector<int> score_linear_input_indexes;
	std::vector<double> score_linear_weights;

	std::vector<std::vector<int>> score_network_input_indexes;
	Network* score_network;

	/**
	 * - difference between start and final
	 *   - end not involved
	 *   - for learning XORs
	 */
	double vs_average_score;

	std::vector<bool> vs_input_is_start;
	std::vector<AbstractNode*> vs_input_node_contexts;
	std::vector<int> vs_input_obs_indexes;

	std::vector<int> vs_linear_input_indexes;
	std::vector<double> vs_linear_weights;

	std::vector<std::vector<int>> vs_network_input_indexes;
	Network* vs_network;

	AbstractExperiment* experiment;

	Eval(Scope* parent_scope);
	Eval(Eval* original,
		 Solution* parent_solution);
	~Eval();

	void activate(Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory*& scope_history);
	double calc_score(RunHelper& run_helper,
					  ScopeHistory* scope_history);
	double calc_vs(RunHelper& run_helper,
				   EvalHistory* history);

	void init();
	void load(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

class EvalHistory {
public:
	Eval* eval;

	ScopeHistory* start_scope_history;
	ScopeHistory* end_scope_history;

	EvalHistory(Eval* eval);
	EvalHistory(EvalHistory* original);
	~EvalHistory();
};

#endif /* EVAL_H */