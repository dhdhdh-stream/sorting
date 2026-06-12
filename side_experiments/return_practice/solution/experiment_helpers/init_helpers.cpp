/**
 * - need long-trained network for end network
 *   - need long-trained to predict crazy correctly
 * 
 * TODO: instead of predicting until end, predict until signal end of scope
 */

#include "experiment.h"

#include "constants.h"
#include "globals.h"

using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int NUM_SIMULATE = 40;
// #else
// const int NUM_SIMULATE = 4000;
// #endif /* MDEBUG */

// #if defined(MDEBUG) && MDEBUG
// const int TRAIN_ITERS = 30;
// #else
// const int TRAIN_ITERS = 300000;
// #endif /* MDEBUG */

// #if defined(MDEBUG) && MDEBUG
// const int NUM_EXPLORE = 5;
// #else
// const int NUM_EXPLORE = 1000;
// #endif /* MDEBUG */

void init_experiment_helper(AbstractNode* node_context,
							bool is_branch,
							Wrapper* wrapper) {
	// uniform_int_distribution<int> start_distribution(0, STATE_NUM_SAVE-1);
	// uniform_int_distribution<int> train_distribution(0, NUM_SIMULATE-1);


}
