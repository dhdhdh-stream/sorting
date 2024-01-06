/**
 * - TODO: use tries to guide what to explore
 *   - "what if I take this try, but swap order/add step/remove step"
 */

// score is reliable
// - information is not

// but how good pass through/branch are depends just as much on information as score

// least squares is reliable, NNs are not

// so if no state added, then reliable

// new state is like bonus
// - probably don't take into account

// start by merging branch and passthrough
// - so goes:
//   - existing
//   - new
//     - pass through
//     - branch
//     - information

// to create, either random select, or targeted
// - if random, use current branch criteria (so apply for pass through as well)

// actually, not easy to merge branch and pass through
// - so track and random/target separately?

// branch cares about combined score?
// - and branch_weight partially
// pass through cares about new score and new information?

// start from success and move around
// - look for things 0.7 geometric away

// sometimes, look for actions that aren't explored by looking at counts

// other times, look for potential
// - keep track of, like, last 200 to prevent retries

// can clean nodes in scopes by making them no-ops?

// need to better capture scope creation process
// - need to be able to make minor variations

// don't make new scopes permanent
// - don't connect state all the way to the bottom
//   - pull from and set back to outer
// - then try small variations
//   - variations connect to variations
//     - if fail, then connect to stub

// edit outer context state in place
// - so action nodes need to go through one layer of indirection

// actually, no difference between scope nodes and scopes?
// - or at least, 1-to-1 connection between scope nodes and scopes

#ifndef TRY_TRACKER_H
#define TRY_TRACKER_H

#include <fstream>
#include <map>
#include <utility>
#include <vector>

class TryExitImpact;
class TryImpact;
class TryInstance;
class TryStartImpact;

const int TRY_INSERT = 0;
const int TRY_REMOVE = 1;
const int TRY_SUBSTITUTE = 2;

class TryTracker {
public:
	std::vector<TryInstance*> tries;

	std::map<std::pair<int, int>, TryImpact*> action_impacts;
	std::map<std::pair<int, std::pair<int,int>>, TryImpact*> node_impacts;

	std::map<std::pair<int, std::pair<std::vector<int>,std::vector<int>>>, TryStartImpact*> start_impacts;
	std::map<std::pair<int, std::pair<std::vector<int>,std::vector<int>>>, TryExitImpact*> exit_impacts;

	TryTracker();
	~TryTracker();

	void evaluate_potential(TryInstance* potential,
							double& predicted_impact,
							double& closest_distance);
	void update(TryInstance* new_add);
};

#endif /* TRY_TRACKER_H */