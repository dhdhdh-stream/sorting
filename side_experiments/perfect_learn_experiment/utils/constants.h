#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const int NUM_STATES = 16;

const double SCORE_LEARNING_RATE = 0.001;
const double STATE_LEARNING_RATE = 0.0002;

const int INIT_EPOCH_SIZE = 10;
const int UPDATE_EPOCH_SIZE = 100;

#endif /* CONSTANTS_H */