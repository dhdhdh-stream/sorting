#ifndef CRAZY_H
#define CRAZY_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExperimentRun;
class PredictRun;

class Crazy : public AbstractExperiment {
public:
	AbstractNode* node_context;
	bool is_branch;

	std::vector<int> actions;
	AbstractNode* exit_next_node;

	~Crazy();

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);

	void predict_activate(PredictRun* run);
	void predict_step(PredictRun* run);
};

class CrazyState : public AbstractExperimentState {
public:
	int step_index;

	CrazyState(Crazy* experiment);
};

#endif /* CRAZY_H */