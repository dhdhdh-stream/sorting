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
#include "signal.h"

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

	std::vector<int> impacted_factors;

	double average_hits_per_run;
	double average_instances_per_run;

	int last_updated_run_index;
	int sum_hits;
	int sum_instances;

	ScopeNode();
	ScopeNode(ScopeNode* original,
			  Solution* parent_solution);

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);
	void exit_step(SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_exit_step(SolutionWrapper* wrapper);

	void clean();
	void measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ScopeNodeHistory : public AbstractNodeHistory {
public:
	ScopeHistory* scope_history;

	ScopeNodeHistory(ScopeNode* node);
	ScopeNodeHistory(ScopeNodeHistory* original);
	~ScopeNodeHistory();
};

#endif /* SCOPE_NODE_H */