/**
 * - add to later ObsNode
 * 
 * TODO:
 * - add across scopes
 *   - add to ScopeNodes
 *     - specifically, add to later of ObsNode/ScopeNode
 */

#ifndef MATCH_H
#define MATCH_H

#include <fstream>
#include <vector>

#include "run_helper.h"

class ObsNode;
class Scope;
class ScopeHistory;
class Solution;

class Match {
public:
	ObsNode* parent;

	std::vector<Scope*> scope_context;
	std::vector<int> node_context;

	double weight;
	double constant;

	double standard_deviation;

	double average_distance;

	bool is_init;
	std::vector<std::pair<double,double>> datapoints;
	std::vector<int> distances;

	Match();
	Match(std::ifstream& input_file,
		  Solution* parent_solution);

	bool should_delete(Scope* scope,
					   int node_id);
	bool should_delete(Scope* scope);

	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id);

	void clean();

	void update(bool& is_still_needed);

	void save(std::ofstream& output_file);
};

#endif /* MATCH_H */