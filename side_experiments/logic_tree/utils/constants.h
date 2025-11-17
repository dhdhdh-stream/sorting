#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int SPLIT_TYPE_GREATER = 0;
const int SPLIT_TYPE_GREATER_EQUAL = 1;
const int SPLIT_TYPE_LESSER_EQUAL = 2;
const int SPLIT_TYPE_LESSER = 3;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int MEASURE_ITERS = 40;
#else
const int MEASURE_ITERS = 4000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */