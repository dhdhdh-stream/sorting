#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"

class WorldModelWrapper;
class Wrapper;

class ActionNode : public AbstractNode {
public:
	int action;

	int next_node_id;
	AbstractNode* next_node;

	ActionNode();

	void step(int& action,
			  bool& is_next,
			  Run* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
	void predict_step(PredictRun* run);

	void save(std::ofstream& output_file,
			  Wrapper* wrapper);
	void load(std::ifstream& input_file);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	ActionNodeHistory(ActionNode* node);
};

#endif /* ACTION_NODE_H */