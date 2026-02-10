#ifndef CONSTANTS_H
#define CONSTANTS_H

const double MIN_WEIGHT = 0.00001;
const double MIN_STANDARD_DEVIATION = 0.00001;

const double NETWORK_TARGET_MAX_UPDATE = 0.01;
const int NETWORK_EPOCH_SIZE = 20;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

#endif /* CONSTANTS_H */