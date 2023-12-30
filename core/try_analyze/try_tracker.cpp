#include "try_tracker.h"

using namespace std;



void TryTracker::evaluate_potential(TryInstance* potential,
									TryInstance*& closest_match,
									double& predicted_impact,
									vector<pair<int, pair<int, int>>>& closest_diffs) {
	int best_index;
	double best_distance = numeric_limits<double>::max();
	vector<pair<int, pair<int, int>>> best_diffs;
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		int distance;
		vector<pair<int, pair<int, int>>> diffs;
		try_distance(this->tries[t_index],
					 potential,
					 distance,
					 diffs);

		if (distance < best_distance) {
			best_index = t_index;
			best_distance = distance;
			best_diffs = diffs;
		}
	}

	closest_match = this->tries[best_index];

	int num_impacts = 0;
	double sum_impacts = 0.0;
	for (int d_index = 0; d_index < (int)best_diffs.size(); d_index++) {
		if (best_diffs[d_index] == TRY_INSERT) {
			if (potential->step_types[best_diffs[d_index].second.second] == STEP_TYPE_ACTION) {
				int action = potential->actions[best_diffs[d_index].second.second];

				map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
				if (it != this->action_impacts.end()) {

				}
			} else {

			}
		} else if (best_diffs[d_index] == TRY_REMOVE) {

		} else {
			// best_diffs[d_index] == TRY_SUBSTITUTE

		}
	}


}
