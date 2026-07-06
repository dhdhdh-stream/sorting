#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

const double NETWORK_INIT_MULTIPLIER = 0.01;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int EPOCH_SIZE = 10;
/**
 * - not meaningful to update weights more often(?)
 */

const int HIDDEN_1_STATE_SIZE_MULTIPLE = 4;
const int HIDDEN_2_STATE_SIZE_MULTIPLE = 2;
const int HIDDEN_3_STATE_SIZE_MULTIPLE = 1;

#endif /* CONSTANTS_H */