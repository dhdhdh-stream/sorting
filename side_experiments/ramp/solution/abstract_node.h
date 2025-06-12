#ifndef ABSTRACT_NODE_H
#define ABSTRACT_NODE_H

#include <fstream>
#include <vector>

class AbstractExperiment;
class Scope;
class Solution;

const int NODE_TYPE_ACTION = 0;
const int NODE_TYPE_SCOPE = 1;
const int NODE_TYPE_BRANCH = 2;
const int NODE_TYPE_OBS = 3;

class AbstractNode {
public:
	int type;

	Scope* parent;
	int id;

	std::vector<int> ancestor_ids;
	/**
	 * - if both paths of BranchNode point to same node, add twice
	 */

	AbstractExperiment* experiment;

	int is_init;
	AbstractExperiment* init_experiment;

	virtual ~AbstractNode() {};

	virtual void clean_inputs(Scope* scope,
							  int node_id) = 0;
	virtual void clean_inputs(Scope* scope) = 0;
	virtual void replace_factor(Scope* scope,
								int original_node_id,
								int original_factor_index,
								int new_node_id,
								int new_factor_index) = 0;
	virtual void replace_obs_node(Scope* scope,
								  int original_node_id,
								  int new_node_id) = 0;
	virtual void replace_scope(Scope* original_scope,
							   Scope* new_scope,
							   int new_scope_node_id) = 0;

	virtual void save(std::ofstream& output_file) = 0;
	virtual void link(Solution* parent_solution) = 0;
	virtual void save_for_display(std::ofstream& output_file) = 0;
};

class AbstractNodeHistory {
public:
	AbstractNode* node;

	int index;

	virtual ~AbstractNodeHistory() {};
};

#endif /* ABSTRACT_NODE_H */