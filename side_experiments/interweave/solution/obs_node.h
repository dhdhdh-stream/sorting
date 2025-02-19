#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"
#include "run_helper.h"

class Factor;
class Problem;
class Scope;
class ScopeHistory;
class Solution;

class ObsNodeHistory;
class ObsNode : public AbstractNode {
public:
	bool is_used;

	std::vector<Factor*> factors;

	int next_node_id;
	AbstractNode* next_node;

	ObsNode();
	~ObsNode();

	void activate(AbstractNode*& curr_node,
				  Problem* problem,
				  RunHelper& run_helper,
				  ScopeHistory* scope_history);

	void measure_activate(AbstractNode*& curr_node,
						  Problem* problem,
						  RunHelper& run_helper,
						  ScopeHistory* scope_history);

	void clear_experiments();

	void clean_inputs(Scope* scope,
					  int node_id);
	void clean_inputs(Scope* scope);

	void unlink();
	void relink();

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link();
	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	std::vector<bool> factor_initialized;
	std::vector<double> factor_values;

	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */