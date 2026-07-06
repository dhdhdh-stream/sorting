#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <fstream>
#include <map>
#include <vector>

#include "abstract_node.h"

class Network;
class Problem;
class ScopeHistory;
class SolutionWrapper;

class ActionNodeHistory;
class ActionNode : public AbstractNode {
public:
	int action;

	int next_node_id;
	AbstractNode* next_node;

	Network* network;
	std::vector<std::vector<double>> sample_obs;
	std::vector<double> sample_target_vals;
	int sample_index;

	double average_instances_per_run;
	AbstractExperiment* experiment;

	int curr_instances_per_run;

	ActionNode();
	~ActionNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);
	void experiment_step_callback(std::vector<double>& obs,
								  SolutionWrapper* wrapper);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);

	void copy_from(ActionNode* original);

	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> obs;

	ActionNodeHistory(ActionNode* node);
};

#endif /* ACTION_NODE_H */