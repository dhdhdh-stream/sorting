#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"

class WorldModelWrapper;

class ActionNode : public AbstractNode {
public:
	int action;

	int next_node_id;
	AbstractNode* next_node;



	void experiment_step(std::vector<double>& obs,
						 int& action,
						 bool& is_next,
						 ExperimentRun& run);
	void predict_step(PredictRun& run);


};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	ActionNodeHistory(ActionNode* node);
};

#endif /* ACTION_NODE_H */