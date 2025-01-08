#ifndef FACTOR_H
#define FACTOR_H

#include <fstream>
#include <utility>
#include <vector>

#include "context_layer.h"
#include "run_helper.h"

class Network;
class ObsNode;
class Scope;
class ScopeHistory;
class Solution;

class Factor {
public:
	std::vector<std::pair<std::pair<std::vector<Scope*>,std::vector<int>>,
		std::pair<int,int>>> inputs;
	Network* network;

	std::vector<std::vector<Scope*>> input_scope_contexts;
	std::vector<std::vector<int>> input_node_context_ids;

	Factor();
	Factor(Factor* original,
		   Solution* parent_solution);
	~Factor();

	void activate(int index,
				  std::vector<ContextLayer>& context,
				  RunHelper& run_helper,
				  bool& initialized,
				  double& value);

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