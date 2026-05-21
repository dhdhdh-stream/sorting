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

	Experiment* experiment;



	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun& run);
	void predict_step(PredictRun& run);


};

class ObsNodeHistory : public AbstractNodeHistory {
public:
	ObsNodeHistory(ObsNode* node);
};

#endif /* OBS_NODE_H */