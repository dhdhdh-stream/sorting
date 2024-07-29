/**
 * - perhaps learn permanent world model instead?
 *   - creating and accounting for all possibilities off 1 run not feasible?
 *   - or at least structure of world model fixed
 *     - obs can change
 *       - and help with localization
 * - perhaps mix of permanent and temporary
 * 
 * - or, there are actions that can be taken that greatly, greatly reduce uncertainty
 *   - do those need to be learned?
 *   - should these be performed consistently?
 * - or actually, maybe only when actions are relatively deterministic
 * 
 * - also, if have greater vision and can clearly identify when state is the same helps
 * 
 * - maybe if too much uncertainty (in actions and obs), simply cannot make world model?
 *   - too easy to lose way to known, too hard to identify within known
 *   - so have to assume world is somewhat sane?
 *   - or at least, the features have to be bigger
 *     - like, actions and obs can both be uncertain, but feature so big that on average there will still be good data
 * 
 * - or at least, maybe large parts of the world can be uncertain, but there are signposts
 *   - areas which are clearly distinct
 * 
 * - perhaps problems are created around world model
 *   - starting and ending from the same spot
 * 
 * - is it possible to know what you want without the steps to achieve it?
 *   - could be that not everywhere explored yet
 *     - or even explored consistently in a way that can identify average improvement
 *     - so what you want may not be what is best
 *   - could also be that what you want is lottery
 *     - good when everything aligns, bad on average
 *     - so actually detrimental
 *   - could also be that what you want is a combination of many many things
 *     - difficult to target
 *     - improving on something might cost something else
 *     - so what you want is to do something that's generally good, rather than something super specific
 * 
 * - with world model, can directly head to locations
 *   - don't need nodes to return to
 * 
 * - perhaps traveling to location is a subproblem?
 * 
 * TODO: don't try to localize through individual squares, but based on regions
 * - though can mix in some sharp features
 * - maybe gradually build out like a Markov model?
 *   - probability of travelling between states
 *     - initially, few distinct states
 *       - but gradually, duplicate-ish states that represent distance
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

#include "action_helpers.h"
#include "constants.h"
#include "predicted_world.h"
#include "world_truth.h"

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;

	WorldTruth world_truth;
	PredictedWorld predicted_world;

	uniform_int_distribution<int> action_distribution(0, 3);
	for (int iter_index = 0; iter_index < 10; iter_index++) {
		int action = action_distribution(generator);

		int new_obs;
		apply_action(world_truth,
					 action,
					 new_obs);

		cout << iter_index << endl;
		cout << "action: " << action << endl;
		cout << "new_obs: " << new_obs << endl;

		update_predicted(predicted_world,
						 action,
						 new_obs);

		cout << "predicted_world.possible_models.size(): " << predicted_world.possible_models.size() << endl;
		cout << endl;
	}

	cout << "world_truth:" << endl;
	world_truth.print();

	cout << "Done" << endl;
}
