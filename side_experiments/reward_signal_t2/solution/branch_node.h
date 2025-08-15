#ifndef BRANCH_NODE_H
#define BRANCH_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "input.h"

class Problem;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class BranchNodeHistory;
class BranchNode : public AbstractNode {
public:
	double average_val;
	std::vector<Input> inputs;
	std::vector<double> input_averages;
	std::vector<double> input_standard_deviations;
	std::vector<double> weights;

	int original_next_node_id;
	AbstractNode* original_next_node;
	int branch_next_node_id;
	AbstractNode* branch_next_node;

	std::vector<int> impacted_factors;

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	double original_average_hits_per_run;
	double original_average_instances_per_run;
	double branch_average_hits_per_run;
	double branch_average_instances_per_run;

	int original_last_updated_run_index;
	int original_sum_hits;
	int original_sum_instances;
	int branch_last_updated_run_index;
	int branch_sum_hits;
	int branch_sum_instances;

	BranchNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	#if defined(MDEBUG) && MDEBUG
	void verify_step(std::vector<double>& obs,
					 int& action,
					 bool& is_next,
					 SolutionWrapper* wrapper);
	void clear_verify();
	#endif /* MDEBUG */

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void clean();
	void measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class BranchNodeHistory : public AbstractNodeHistory {
public:
	bool is_branch;

	BranchNodeHistory(BranchNode* node);
	BranchNodeHistory(BranchNodeHistory* original);
};

#endif /* BRANCH_NODE_H */