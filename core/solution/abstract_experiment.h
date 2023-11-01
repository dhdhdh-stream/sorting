#ifndef ABSTRACT_EXPERIMENT_H
#define ABSTRACT_EXPERIMENT_H

const int EXPERIMENT_TYPE_BRANCH = 0;
const int EXPERIMENT_TYPE_PASS_THROUGH = 1;

class AbstractExperiment {
public:
	int type;

	virtual void activate(int& curr_node_id,
						  Problem& problem,
						  vector<ContextLayer>& context,
						  int& exit_depth,
						  int& exit_node_id,
						  RunHelper& run_helper,
						  AbstractExperimentHistory*& history);

};

class AbstractExperimentHistory {
public:
	AbstractExperiment* experiment;


};

#endif /* ABSTRACT_EXPERIMENT_H */