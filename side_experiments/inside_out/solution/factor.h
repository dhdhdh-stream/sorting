#ifndef FACTOR_H
#define FACTOR_H

#include <fstream>
#include <list>
#include <utility>
#include <vector>

#include "input.h"

class Network;
class ObsNode;
class Scope;
class ScopeHistory;
class Solution;

class Factor {
public:
	std::vector<Input> inputs;
	Network* network;

	Factor();
	Factor(Factor* original,
		   Solution* parent_solution);
	~Factor();

	double back_activate(ScopeHistory* scope_history);

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

#endif /* FACTOR_H */