#include "state_network.h"

using namespace std;

/**
 * - practical compromise
 *   - increasing decreases speed, but improves likelihood of not getting stuck
 */
const int STATE_NETWORK_HIDDEN_SIZE = 20;

const double STATE_NETWORK_TARGET_MAX_UPDATE = 0.05;

