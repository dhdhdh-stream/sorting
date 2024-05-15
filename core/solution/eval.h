/**
 * - don't worry about history
 *   - if tied to scope, then won't be worrying about long past history
 *     - so simply also don't worry about recent history
 * 
 * - target_val is made up of that which is predictable and that which isn't
 *   - by using eval, will remove impact of that which isn't predictable
 *     - reducing variance, improving training
 *   - but wil lose that which isn't yet predictable by eval
 *     - making it difficult to find sequences that lead to positive but new/strange situations
 * 
 * - run eval both at start and at end
 *   - to account for different possible starting points
 *     - greatly reducing variance
 *   - when training start, target_val is final target_val
 *   - when training end, target_val is diff between starting predicted score and final target_val
 *   - when making additions, add a sequence to start and a sequence to end together
 */

#ifndef EVAL_H
#define EVAL_H

#include <fstream>
#include <vector>

#include "run_helper.h"

class AbstractNode;
class Network;
class Problem;
class Scope;
class Solution;

class EvalHistory;
class Eval {
public:
	Scope* parent_scope;

	Scope* start_subscope;
	Scope* end_subscope;

	double start_average_score;

	std::vector<AbstractNode*> start_input_node_contexts;
	std::vector<int> start_input_obs_indexes;

	std::vector<int> start_linear_input_indexes;
	std::vector<double> start_linear_weights;

	std::vector<std::vector<int>> start_network_input_indexes;
	Network* start_network;

	double end_average_score;

	std::vector<bool> end_is_start;
	std::vector<AbstractNode*> end_input_node_contexts;
	std::vector<int> end_input_obs_indexes;

	std::vector<int> end_linear_input_indexes;
	std::vector<double> end_linear_weights;

	std::vector<std::vector<int>> end_network_input_indexes;
	Network* end_network;

	AbstractExperiment* experiment;

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
	double calc_impact(RunHelper& run_helper,
					   EvalHistory* history);

	void experiment_activate(Problem* problem,
							 RunHelper& run_helper);

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
	~EvalHistory();
};

#endif /* EVAL_H */