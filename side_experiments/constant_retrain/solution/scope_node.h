/**
 * - don't bother with hooks into inner
 *   - quite messy when multiple hooks on same node
 *     - if try one by one, cannot override early mistake
 *     - if only use one outer, restrictive
 *   - simply rely on inner's child scopes instead
 */

#ifndef SCOPE_NODE_H
#define SCOPE_NODE_H

#include <fstream>
#include <map>
#include <set>
#include <utility>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class ScopeNodeHistory;
class ScopeNode : public AbstractNode {
public:
	Scope* scope;

	int next_node_id;
	AbstractNode* next_node;

	Network* network;
	std::vector<std::vector<double>> sample_obs;
	std::vector<double> sample_target_vals;
	int sample_index;

	double average_instances_per_run;
	AbstractExperiment* experiment;

	int curr_instances_per_run;

	ScopeNode();
	~ScopeNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void exit_step(SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(std::vector<double>& obs,
							  SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);

	void copy_from(ScopeNode* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	std::vector<double> obs;

	ScopeNodeHistory(ScopeNode* node);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */