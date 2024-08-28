#ifndef KEY_POINT_HELPERS_H
#define KEY_POINT_HELPERS_H

#include <vector>

class KeyPoint;
class WorldTruth;

KeyPoint* create_potential_key_point(WorldTruth* world_truth);

bool find_paths_for_potential(WorldTruth* world_truth,
							  KeyPoint* potential,
							  std::vector<std::vector<double>>& path_obs,
							  std::vector<std::vector<int>>& path_actions);

bool verify_potential_uniqueness(WorldTruth* world_truth,
								 KeyPoint* potential,
								 std::vector<std::vector<int>>& path_actions);

#endif /* KEY_POINT_HELPERS_H */