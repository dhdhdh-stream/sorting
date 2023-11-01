#ifndef OUTER_EXPERIMENT_H
#define OUTER_EXPERIMENT_H

const int OUTER_EXPERIMENT_STATE_EXPLORE = 0;
const int OUTER_EXPERIMENT_STATE_MEASURE_SCORE = 1;
const int OUTER_EXPERIMENT_STATE_TRAIN = 2;
const int OUTER_EXPERIMENT_STATE_MEASURE_MISGUESS = 3;
const int OUTER_EXPERIMENT_STATE_EXPERIMENT = 4;

const int OUTER_EXPERIMENT_STATE_FAIL = 5;
const int OUTER_EXPERIMENT_STATE_SUCCESS = 6;

class OuterExperiment {
public:
	/**
	 * - set on initialize
	 */
	double existing_average_score;
	double existing_average_misguess;



};

#endif /* OUTER_EXPERIMENT_H */