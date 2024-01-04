/**
 * - TODO: use tries to guide what to explore
 *   - "what if I take this try, but swap order/add step/remove step"
 */

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