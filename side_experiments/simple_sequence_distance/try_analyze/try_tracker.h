#ifndef TRY_TRACKER_H
#define TRY_TRACKER_H

#include <map>
#include <utility>
#include <vector>

class Try;
class TryImpact;

const int TRY_INSERT = 0;
const int TRY_REMOVE = 1;

class TryTracker {
public:
	std::vector<Try*> tries;

	/**
	 * - (type, action)
	 */
	std::map<std::pair<int, int>, TryImpact*> impacts;

	TryTracker();
	~TryTracker();

	void evaluate_potential(Try* potential,
							Try*& closest_match,
							double& predicted_impact,
							std::vector<std::pair<int, int>>& closest_diffs);
	void backprop(Try* new_add,
				  Try* closest_match,
				  double predicted_impact,
				  std::vector<std::pair<int, int>>& closest_diffs);
};

#endif /* TRY_TRACKER_H */