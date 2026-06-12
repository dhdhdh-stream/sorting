#ifndef ACTION_NODE_H
#define ACTION_NODE_H

#include <vector>

#include "abstract_node.h"

class AbstractExperiment;
class Experiment;
class WorldModelWrapper;
class Wrapper;

class ActionNode : public AbstractNode {
public:
	int action;

	int next_node_id;
	AbstractNode* next_node;

	std::vector<std::vector<double>> state_history;
	int history_index;
	AbstractExperiment* experiment;

	ActionNode();

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
	void load(std::ifstream& input_file);
	void link(Wrapper* wrapper);
	void save_for_display(std::ofstream& output_file);
};

class ActionNodeHistory : public AbstractNodeHistory {
public:
	std::vector<double> state;
	std::vector<double> large_state;

	ActionNodeHistory(ActionNode* node);
};

#endif /* ACTION_NODE_H */