#ifndef BRANCH_EXPERIMENT_H
#define BRANCH_EXPERIMENT_H

const int BRANCH_EXPERIMENT_STEP_TYPE_ACTION = 0;
const int BRANCH_EXPERIMENT_STEP_TYPE_SEQUENCE = 1;

class BranchExperiment {
public:
	int num_steps;
	std::vector<int> step_types;
	// std::vector<Action> actions;
	std::vector<Sequence*> sequences;

	int exit_depth;
	int exit_node_id;



}

#endif /* BRANCH_EXPERIMENT_H */