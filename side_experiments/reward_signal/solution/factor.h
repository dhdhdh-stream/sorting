#ifndef FACTOR_H
#define FACTOR_H

#include <fstream>
#include <utility>
#include <vector>

#include "input.h"

class Network;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class Factor {
public:
	std::vector<Input> inputs;
	Network* network;

	std::vector<int> impacted_factors;

	bool is_meaningful;

	Factor();
	~Factor();

	double back_activate(ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);
	void replace_obs_node(Scope* scope,
						  int original_node_id,
						  int new_node_id,
						  int index);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(int index);
};

#endif /* FACTOR_H */