#ifndef KEYPOINT_HELPERS_H
#define KEYPOINT_HELPERS_H

#include <vector>

class Keypoint;
class WorldTruth;

Keypoint* create_potential_keypoint(WorldTruth* world_truth);

bool find_paths_for_potential(WorldTruth* world_truth,
							  Keypoint* potential,
							  std::vector<std::vector<double>>& path_obs,
							  std::vector<std::vector<int>>& path_actions);

bool verify_potential_uniqueness(WorldTruth* world_truth,
								 Keypoint* potential,
								 std::vector<std::vector<int>>& path_actions);

#endif /* KEYPOINT_HELPERS_H */