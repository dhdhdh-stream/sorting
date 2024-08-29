/**
 * TODO:
 * - to handle variation, identify keypoints which are at the same location
 *   - then find commonalities and merge
 * - merge match for multiple KeyPoints
 */

#ifndef KEYPOINT_H
#define KEYPOINT_H

#include <fstream>
#include <vector>

class WorldTruth;

class Keypoint {
public:
	std::vector<double> obs;
	std::vector<int> actions;
	/**
	 * - actions.size() == obs.size() + 1
	 */

	Keypoint();

	bool match(WorldTruth* world_truth);
	bool match(WorldTruth* world_truth,
			   std::vector<int>& unknown_actions,
			   std::vector<double>& unknown_obs);

	void save(std::ofstream& output_file);
	void load(std::ifstream& input_file);
};

#endif /* KEYPOINT_H */