#ifndef START_NODE_H
#define START_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class AbstractExperiment;

class StartNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	double average_instances_per_run;
	std::vector<std::vector<double>> state_history;
	int history_index;
	AbstractExperiment* experiment;

	int curr_instances_per_run;

	StartNode();
	~StartNode();

	void step(int& action,
			  bool& is_next,
			  Run* run);

	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void experiment_step_start(ExperimentRun* run);

	void predict_step(PredictRun* run);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file,
			  Wrapper* wrapper);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class StartNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> state;

	StartNodeHistory(StartNode* node);
};

#endif /* START_NODE_H */