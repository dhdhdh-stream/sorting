#include "world_model_helpers.h"

#include "keypoint.h"
#include "world_model.h"

using namespace std;

bool potential_keypoint_is_duplicate(WorldModel* world_model,
									 Keypoint* potential) {
	for (int k_index = 0; k_index < (int)world_model->keypoints.size(); k_index++) {
		Keypoint* existing = world_model->keypoints[k_index];
		if (existing->obs.size() == potential->obs.size()
				&& existing->actions.size() == potential->actions.size()) {
			bool is_match = true;

			for (int o_index = 0; o_index < (int)potential->obs.size(); o_index++) {
				if (existing->obs[o_index] != potential->obs[o_index]) {
					is_match = false;
					break;
				}
			}

			for (int a_index = 0; a_index < (int)potential->actions.size(); a_index++) {
				if (existing->actions[a_index] != potential->actions[a_index]) {
					is_match = false;
					break;
				}
			}

			if (is_match) {
				return true;
			}
		}
	}

	return false;
}
