#ifndef NOOP_NODE_H
#define NOOP_NODE_H

#include <fstream>
#include <utility>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class ScopeHistory;
class ScoreNetwork;
class Solution;
class SolutionWrapper;

class NoopNodeHistory;
class NoopNode : public AbstractNode {
public:
	ScoreNetwork* score_network;

	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_hit;
	double average_instances_per_run;
	AbstractExperiment* experiment;

	int curr_num_instances;

	NoopNode();
	~NoopNode();

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

class NoopNodeHistory : public AbstractNodeHistory {
public:
	NoopNodeHistory(NoopNode* node);
};

#endif /* NOOP_NODE_H */