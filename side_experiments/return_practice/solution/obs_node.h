#ifndef OBS_NODE_H
#define OBS_NODE_H

#include <fstream>
#include <vector>

#include "abstract_node.h"

class Experiment;

class ObsNode : public AbstractNode {
public:
	int next_node_id;
	AbstractNode* next_node;

	std::vector<std::vector<double>> state_history;
	int state_history_index;

	double average_instances_per_run;

	Experiment* experiment;

	int sum_instances;

	ObsNode();
	~ObsNode();

	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void predict_step(PredictRun* run);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file,
			  Wrapper* wrapper);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */