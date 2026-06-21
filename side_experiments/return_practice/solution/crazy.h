#ifndef CRAZY_H
#define CRAZY_H

#include <vector>

#include "abstract_experiment.h"

class AbstractNode;
class ExperimentRun;

void create_crazy_helper(AbstractNode* node_context,
						 bool is_branch,
						 ExperimentRun* run);

class Crazy : public AbstractExperiment {
public:
	std::vector<int> actions;
	AbstractNode* exit_next_node;

	void experiment_activate(ExperimentRun* run);
	void experiment_step(int& action,
						 bool& is_next,
						 ExperimentRun* run);
};

class CrazyState : public AbstractExperimentState {
public:
	int step_index;

	CrazyState(Crazy* crazy);
};

#endif /* CRAZY_H */