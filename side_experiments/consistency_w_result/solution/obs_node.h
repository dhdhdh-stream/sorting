#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "match.h"
#include "run_helper.h"

class Factor;
class Keypoint;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	std::vector<Factor*> factors;

	int next_node_id;
	AbstractNode* next_node;

	/**
	 * - remeasure, but don't look for new matches
	 *   - only remove ones that no longer match
	 */
	double average;
	double standard_deviation;
	bool is_fixed_point;
	std::vector<Match> matches;

	double sum_obs_average;
	double sum_obs_variance;
	int obs_count;

	bool is_match_start;

	ObsNode();
	ObsNode(ObsNode* original,
			Solution* parent_solution);
	~ObsNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void result_activate(AbstractNode*& curr_node,
						 Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);
	void experiment_activate(AbstractNode*& curr_node,
							 Problem* problem,
							 RunHelper& run_helper,
							 ScopeHistory* scope_history);

	void commit_activate(Problem* problem,
						 RunHelper& run_helper,
						 ScopeHistory* scope_history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history);

	void measure_match_activate(AbstractNode*& curr_node,
								Problem* problem,
								RunHelper& run_helper,
								ScopeHistory* scope_history);

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
	void measure_match_update();

	void gather_match_datapoints(ObsNodeHistory* history,
								 ScopeHistory* scope_history);
	void update_matches();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	int num_actions;

	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */