#ifndef NOOP_NODE_H
#define NOOP_NODE_H

#include <fstream>
#include <utility>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class ScopeHistory;
class Solution;
class SolutionWrapper;

class NoopNodeHistory;
class NoopNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	Network* network;
	std::vector<std::vector<double>> sample_obs;
	std::vector<double> sample_target_vals;
	int sample_index;

	double average_instances_per_run;
	AbstractExperiment* experiment;

	int curr_instances_per_run;

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

	void copy_from(NoopNode* original,
				   Solution* parent_solution);

	void save_for_display(std::ofstream& output_file);
};

class NoopNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs;

	NoopNodeHistory(NoopNode* node);
};

#endif /* NOOP_NODE_H */