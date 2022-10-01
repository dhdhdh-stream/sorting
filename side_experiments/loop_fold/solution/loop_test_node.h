#ifndef LOOP_TEST_NODE_H
#define LOOP_TEST_NODE_H

#include <vector>

#include "fold_network.h"
#include "compression_network.h"
#include "node.h"
#include "score_network.h"
#include "state_network.h"

const int STATE_LEARN_SCORE = 0;
const int STATE_JUST_SCORE_LEARN = 1;
const int STATE_JUST_SCORE_MEASURE = 2;
const int STATE_JUST_SCORE_TUNE = 3;
const int STATE_LOCAL_SCOPE_LEARN = 4;
const int STATE_LOCAL_SCOPE_MEASURE = 5;
const int STATE_LOCAL_SCOPE_TUNE = 6;
const int STATE_CAN_COMPRESS_LEARN = 7;
const int STATE_CAN_COMPRESS_MEASURE = 8;
const int STATE_COMPRESS_LEARN = 9;
const int STATE_COMPRESS_MEASURE = 10;
const int STATE_COMPRESS_TUNE = 11;
const int STATE_ADD_SCOPE_LEARN = 12;
const int STATE_ADD_SCOPE_MEASURE = 13;
const int STATE_ADD_SCOPE_TUNE = 14;
const int STATE_DONE = 15;

#endif /* LOOP_TEST_NODE_H */