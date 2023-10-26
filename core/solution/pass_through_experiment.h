#ifndef PASS_THROUGH_EXPERIMENT_H
#define PASS_THROUGH_EXPERIMENT_H

const int PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING = 0;

const int PASS_THROUGH_EXPERIMENT_STATE_EXPLORE = 1;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_SUB_STATE_FIND_POSITIVE = 0;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_SUB_STATE_SCORE_MEASURE = 1;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_SUB_STATE_TRAIN = 2;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_SUB_STATE_MISGUESS_MEASURE = 3;

const int PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT = 2;

class PassThroughExperiment {
public:
	std::vector<int> scope_context;
	std::vector<int> node_context;

	int state;
	int state_iter;
	int sub_state;
	int sub_state_iter;

	double existing_average_score;
	double existing_average_misguess;


};

#endif /* PASS_THROUGH_EXPERIMENT_H */