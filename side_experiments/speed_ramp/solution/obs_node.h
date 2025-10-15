#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class Problem;
class Scope;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class ObsNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	std::vector<int> impacted_factors;

	int num_experiments;

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

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file,
			  Solution* parent_solution);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs_history;

	ObsNodeHistory(ObsNode* node);
	ObsNodeHistory(ObsNodeHistory* original);
};

#endif /* OBS_NODE_H */