/**
 * - Baum-Welch effective when states have different obs
 *   - and when there's no delay between actions and obs
 * 
 * - comparing sequences doesn't suffer from Markov limitations(?)
 *   - but if use HMM as the underlying model, then will suffer from Markov limitations
 * 
 * - maybe model should be connected model, but with path (instead of single state)
 *   - then can process path with like focus NN?
 *     - each state represents a symbol?
 * 
 * - use unknown rather than assume single state initially
 *   - random spot vs. random spot offset by 1 unlikely to be significantly different(?)
 *   - whereas from a fixed starting point may be significantly different
 * 
 * - maybe don't worry about HMM/Baum-Welch at all
 *   - build state through measuring sequences
 *   - then to determine obs (or anything), process the path
 */

/**
 * - updating average_val makes things converge
 *   - with each cycle, the value gets sharper and sharper
 *     - pushing the uncertainty into the transitions
 * 
 * - but if any transitions missing, then average_val will carry the uncertainty
 */

/**
 * - maybe simply allow transitions between nearby states
 *   - and maybe clean if unneeded afterwards?
 */

/**
 * - maybe think in terms of rules?
 *   - while in a certain context, certain rules apply
 *     - even if the details of the world changes
 * 
 * - maybe only makes sense to learn strategy when the rules of the world is known?
 */

/**
 * - need to focus on that all I need from world model is localization
 *   - which includes:
 *     - knowing where you are
 *     - how to travel from A to B
 * 
 * - but there's also different aspects to this?
 *   - "return" could mean:
 *     - return in terms of (x, y) coordinates
 *     - move body back into position
 *     - return object back to position
 *     - if playing a game, return to some virtual state/location within the game
 */

/**
 * - if from A, reached point similar to A, to check if is actually A, repeat steps again
 *   - so probably the first thing to do is to find a returnable spot?
 */

/**
 * - there's like different objects
 * 
 * - there's also relative vs. global position
 * 
 * - there's also state even without obs
 *   - but localization must be somewhat tied to obs?
 *     - at least in like a Kalman filter type of way
 */

/**
 * - obviously, the world never returns to its previous spot
 *   - so just based on correlation then
 *     - and can use correlation to separate out objects?
 */

/**
 * - don't divide into like world and objects
 *   - everything is an object
 *     - like, if you're in a ship travelling in the world, there would be multiple "worlds" to track anyways
 */

/**
 * - actually, not from sequences, but from loops?
 *   - and look for repetition in obs?
 * 
 * - like move to a stable state?
 *   - do stuff then see if return to stable state?
 *   - then build out from stable state?
 * 
 * - if unable to find stable state again, then rebuild?
 * 
 * - there may be things that are shared between runs
 *   - but initially, assume new world?
 * 
 * - once have stable state, remember anything that gets you away and back from stable state
 */

#include <chrono>
#include <iostream>
#include <map>
#include <thread>
#include <random>

using namespace std;

int seed;

default_random_engine generator;

int main(int argc, char* argv[]) {
	cout << "Starting..." << endl;

	seed = (unsigned)time(NULL);
	srand(seed);
	generator.seed(seed);
	cout << "Seed: " << seed << endl;



	cout << "Done" << endl;
}
