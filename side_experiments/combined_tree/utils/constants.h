#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

/**
 * - when there's correlation, weights can get strange values(?)
 */
const double REGRESSION_WEIGHT_LIMIT = 100000.0;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
const int UPDATE_ITERS = 10;
#else
const int TRAIN_ITERS = 300000;
const int UPDATE_ITERS = 100000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */