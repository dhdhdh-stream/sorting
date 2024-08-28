/**
 * TODO:
 * - to handle variation, identify keypoints which are at the same location
 *   - then find commonalities and merge
 */

#ifndef KEY_POINT_H
#define KEY_POINT_H

#include <vector>

class WorldTruth;

class KeyPoint {
public:
	std::vector<double> obs;
	std::vector<int> actions;
	/**
	 * - actions.size() == obs.size() + 1
	 */

	KeyPoint(std::vector<double> obs,
			 std::vector<int> actions);

	bool match(WorldTruth* world_truth);
	bool match(WorldTruth* world_truth,
			   std::vector<int>& unknown_actions,
			   std::vector<double>& unknown_obs);
};

#endif /* KEY_POINT_H */