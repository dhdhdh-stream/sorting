/**
 * - don't worry about history
 *   - if tied to scope, then won't be worrying about long past history
 *     - so simply also don't worry about recent history
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

class Eval {
public:
	Scope* subscope;

	double average_score;

	std::vector<AbstractNode*> input_node_contexts;
	std::vector<int> input_obs_indexes;

	std::vector<int> linear_input_indexes;
	std::vector<double> linear_weights;

	std::vector<std::vector<int>> network_input_indexes;
	Network* network;

	AbstractExperiment* experiment;

	Eval();
	Eval(Eval* original,
		 Solution* parent_solution);
	~Eval();

	double activate(Problem* problem,
					RunHelper& run_helper);
	void experiment_activate(Problem* problem,
							 RunHelper& run_helper);

	void init();
	void load(std::ifstream& input_file);

	void save(std::ofstream& output_file);
};

#endif /* EVAL_H */