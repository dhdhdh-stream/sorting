#ifndef FACTOR_H
#define FACTOR_H

#include <fstream>
#include <utility>
#include <vector>

#include "input.h"
#include "run_helper.h"

class Network;
class ObsNode;
class Scope;
class ScopeHistory;
class Solution;

class Factor {
public:
	bool is_used;

	std::vector<Input> inputs;
	Network* network;

	Factor();
	Factor(Factor* original,
		   Solution* parent_solution);
	~Factor();

	double back_activate(RunHelper& run_helper,
						 ScopeHistory* scope_history);

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
};

#endif /* FACTOR_H */