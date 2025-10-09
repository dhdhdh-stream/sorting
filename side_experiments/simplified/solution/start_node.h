#ifndef START_NODE_H
#define START_NODE_H

#include <fstream>

#include "abstract_node.h"

class StartNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	double average_hits_per_run;
	double average_instances_per_run;
	double average_score;

	int last_updated_run_index;
	double sum_score;
	int sum_hits;
	int sum_instances;

	int num_experiments;

	StartNode();
	~StartNode();

	void step(std::vector<double>& obs,
			  int& action,
			  bool& is_next,
			  SolutionWrapper* wrapper);

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 SolutionWrapper* wrapper);

	void clean();
	void measure_update(int total_count);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
	void link(Solution* parent_solution);
	void save_for_display(std::ofstream& output_file);
};

class StartNodeHistory : public AbstractNodeHistory {
public:
	StartNodeHistory(StartNode* node);
	StartNodeHistory(StartNodeHistory* original);
};

#endif /* START_NODE_H */