/**
 * - without patterns to reduce variance, difficult to make progress on low samples
 * 
 * - assume already 0'd
 */

#ifndef PATTERN_H
#define PATTERN_H

#include <vector>

#include "input.h"

class Network;
class ScopeHistory;

class Pattern {
public:
	std::vector<Input> keypoints;
	Network* keypoint_network;

	std::vector<Input> inputs;
	Network* predict_network;
	double predict_standard_deviation;

	Pattern();
	~Pattern();

	void activate(bool& has_match,
				  double& predicted,
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

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
};

#endif /* PATTERN_H */