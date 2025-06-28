#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class Factor;
class Keypoint;
class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	std::vector<Factor*> factors;

	int next_node_id;
	AbstractNode* next_node;

	double average_hits_per_run;
	double average_instances_per_run;
	double average_score;

	int last_updated_run_index;
	double sum_score;
	int sum_hits;
	int sum_instances;

	double new_scope_average_hits_per_run;
	double new_scope_average_score;

	double new_scope_sum_score;
	int new_scope_sum_count;

	/**
	 * - only for examination
	 */
	std::vector<double> obs_averages;
	std::vector<double> obs_standard_deviations;
	std::vector<std::vector<double>> obs_val_histories;

	ObsNode();
	~ObsNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

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

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	ObsNodeHistory(ObsNode* node);
	ObsNodeHistory(ObsNodeHistory* original);
};

#endif /* OBS_NODE_H */