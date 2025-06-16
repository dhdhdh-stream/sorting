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

	#if defined(MDEBUG) && MDEBUG
	void* verify_key;
	std::vector<double> verify_scores;
	#endif /* MDEBUG */

	double original_average_hits_per_run;
	double original_average_score;
	double branch_average_hits_per_run;
	double branch_average_score;

	int original_last_updated_run_index;
	double original_sum_score;
	int original_sum_count;
	int branch_last_updated_run_index;
	double branch_sum_score;
	int branch_sum_count;

	double original_new_scope_average_hits_per_run;
	double original_new_scope_average_score;
	double branch_new_scope_average_hits_per_run;
	double branch_new_scope_average_score;

	double original_new_scope_sum_score;
	int original_new_scope_sum_count;
	double branch_new_scope_sum_score;
	int branch_new_scope_sum_count;

	BranchNode();
	~BranchNode();

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
	void replace_factor(Scope* scope,
						int original_node_id,
						int original_factor_index,
						int new_node_id,
						int new_factor_index);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);
	void replace_scope(Scope* original_scope,
					   Scope* new_scope,
					   int new_scope_node_id);

	void clean();
	void measure_update();

	void new_scope_clean();
	void new_scope_measure_update(int total_count);

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