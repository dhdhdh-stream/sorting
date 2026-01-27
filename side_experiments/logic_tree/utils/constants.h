#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const int SPLIT_TYPE_GREATER = 0;
const int SPLIT_TYPE_GREATER_EQUAL = 1;
const int SPLIT_TYPE_LESSER = 2;
const int SPLIT_TYPE_LESSER_EQUAL = 3;
const int SPLIT_TYPE_WITHIN = 4;
const int SPLIT_TYPE_WITHIN_EQUAL = 5;
const int SPLIT_TYPE_WITHOUT = 6;
const int SPLIT_TYPE_WITHOUT_EQUAL = 7;
const int SPLIT_TYPE_REL_GREATER = 8;
const int SPLIT_TYPE_REL_GREATER_EQUAL = 9;
const int SPLIT_TYPE_REL_WITHIN = 10;
const int SPLIT_TYPE_REL_WITHIN_EQUAL = 11;
const int SPLIT_TYPE_REL_WITHOUT = 12;
const int SPLIT_TYPE_REL_WITHOUT_EQUAL = 13;
const int SPLIT_TYPE_EQUAL = 14;

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