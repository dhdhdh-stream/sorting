#include "try_tracker.h"

#include "globals.h"
#include "helpers.h"
#include "try.h"
#include "try_impact.h"

using namespace std;

TryTracker::TryTracker() {
	// do nothing
}

TryTracker::~TryTracker() {
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		delete this->tries[t_index];
	}

	for (map<pair<int, int>, TryImpact*>::iterator it = this->impacts.begin();
			it != this->impacts.end(); it++) {
		delete it->second;
	}
}

void TryTracker::evaluate_potential(Try* potential,
									Try*& closest_match,
									double& predicted_impact,
									vector<pair<int, int>>& closest_diffs) {
	int best_index;
	int best_distance = numeric_limits<int>::max();
	vector<pair<int, int>> best_diffs;
	int equal_count = -1;
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		int distance;
		vector<pair<int, int>> diffs;
		compare_tries(this->tries[t_index],
					  potential,
					  distance,
					  diffs);

		if (distance < best_distance) {
			best_index = t_index;
			best_distance = distance;
			best_diffs = diffs;
			equal_count = 1;
		} else if (distance == best_distance) {
			uniform_int_distribution<int> distribution(0, equal_count);
			if (distribution(generator) == 0) {
				best_index = t_index;
				best_distance = distance;
				best_diffs = diffs;
			}
			equal_count++;
		}
	}

	closest_match = this->tries[best_index];

	int num_impacts = 0;
	double sum_impacts = 0.0;
	for (int d_index = 0; d_index < (int)best_diffs.size(); d_index++) {
		if (best_diffs[d_index].first == TRY_INSERT) {
			int action = potential->sequence[best_diffs[d_index].second];

			map<pair<int, int>, TryImpact*>::iterator it = this->impacts.find({TRY_INSERT, action});
			if (it != this->impacts.end()) {
				num_impacts++;
				sum_impacts += it->second->overall_impact;

				for (int a_index = 0; a_index < best_diffs[d_index].second; a_index++) {
					int curr_action = potential->sequence[a_index];
					map<int, pair<int, double>>::iterator rel_it = it->second->rel_pre_impacts.find(curr_action);
					if (rel_it != it->second->rel_pre_impacts.end()) {
						num_impacts++;
						sum_impacts += rel_it->second.second;
					}
				}
				for (int a_index = best_diffs[d_index].second + 1; a_index < (int)potential->sequence.size(); a_index++) {
					int curr_action = potential->sequence[a_index];
					map<int, pair<int, double>>::iterator rel_it = it->second->rel_post_impacts.find(curr_action);
					if (rel_it != it->second->rel_post_impacts.end()) {
						num_impacts++;
						sum_impacts += rel_it->second.second;
					}
				}
			}
		} else {
			int action = closest_match->sequence[best_diffs[d_index].second];

			map<pair<int, int>, TryImpact*>::iterator it = this->impacts.find({TRY_REMOVE, action});
			if (it != this->impacts.end()) {
				num_impacts++;
				sum_impacts += it->second->overall_impact;

				for (int a_index = 0; a_index < best_diffs[d_index].second; a_index++) {
					int curr_action = closest_match->sequence[a_index];
					map<int, pair<int, double>>::iterator rel_it = it->second->rel_pre_impacts.find(curr_action);
					if (rel_it != it->second->rel_pre_impacts.end()) {
						num_impacts++;
						sum_impacts += rel_it->second.second;
					}
				}
				for (int a_index = best_diffs[d_index].second + 1; a_index < (int)closest_match->sequence.size(); a_index++) {
					int curr_action = closest_match->sequence[a_index];
					map<int, pair<int, double>>::iterator rel_it = it->second->rel_post_impacts.find(curr_action);
					if (rel_it != it->second->rel_post_impacts.end()) {
						num_impacts++;
						sum_impacts += rel_it->second.second;
					}
				}
			}
		}
	}

	predicted_impact = closest_match->result;
	if (num_impacts > 0) {
		predicted_impact += sum_impacts / num_impacts;
	}

	closest_diffs = best_diffs;
}

void TryTracker::backprop(Try* new_add,
						  Try* closest_match,
						  double predicted_impact,
						  vector<pair<int, int>>& closest_diffs) {
	double impact_diff = new_add->result - predicted_impact;

	for (int d_index = 0; d_index < (int)closest_diffs.size(); d_index++) {
		if (closest_diffs[d_index].first == TRY_INSERT) {
			int action = new_add->sequence[closest_diffs[d_index].second];

			map<pair<int, int>, TryImpact*>::iterator it = this->impacts.find({TRY_INSERT, action});
			if (it == this->impacts.end()) {
				TryImpact* new_try_impact = new TryImpact();
				new_try_impact->count = 1;
				new_try_impact->overall_impact = impact_diff;
				it = this->impacts.insert({{TRY_INSERT, action}, new_try_impact}).first;
			} else {
				it->second->count++;
				if (it->second->count >= 100) {
					it->second->overall_impact = 0.99*it->second->overall_impact + 0.01*impact_diff;
				} else {
					it->second->overall_impact = (1.0 - 1.0/it->second->count)*it->second->overall_impact + 1.0/it->second->count*impact_diff;
				}
			}

			for (int a_index = 0; a_index < closest_diffs[d_index].second; a_index++) {
				int curr_action = new_add->sequence[a_index];
				map<int, pair<int, double>>::iterator rel_it = it->second->rel_pre_impacts.find(curr_action);
				if (rel_it == it->second->rel_pre_impacts.end()) {
					it->second->rel_pre_impacts[curr_action] = {1, impact_diff};
				} else {
					rel_it->second.first++;
					if (rel_it->second.first >= 100) {
						rel_it->second.second = 0.99*rel_it->second.second + 0.01*impact_diff;
					} else {
						rel_it->second.second = (1.0 - 1.0/rel_it->second.first)*rel_it->second.second + 1.0/rel_it->second.first*impact_diff;
					}
				}
			}
			for (int a_index = closest_diffs[d_index].second + 1; a_index < (int)new_add->sequence.size(); a_index++) {
				int curr_action = new_add->sequence[a_index];
				map<int, pair<int, double>>::iterator rel_it = it->second->rel_post_impacts.find(curr_action);
				if (rel_it == it->second->rel_post_impacts.end()) {
					it->second->rel_post_impacts[curr_action] = {1, impact_diff};
				} else {
					rel_it->second.first++;
					if (rel_it->second.first >= 100) {
						rel_it->second.second = 0.99*rel_it->second.second + 0.01*impact_diff;
					} else {
						rel_it->second.second = (1.0 - 1.0/rel_it->second.first)*rel_it->second.second + 1.0/rel_it->second.first*impact_diff;
					}
				}
			}
		} else {
			int action = closest_match->sequence[closest_diffs[d_index].second];

			map<pair<int, int>, TryImpact*>::iterator it = this->impacts.find({TRY_REMOVE, action});
			if (it == this->impacts.end()) {
				TryImpact* new_try_impact = new TryImpact();
				new_try_impact->count = 1;
				new_try_impact->overall_impact = impact_diff;
				it = this->impacts.insert({{TRY_REMOVE, action}, new_try_impact}).first;
			} else {
				it->second->count++;
				if (it->second->count >= 100) {
					it->second->overall_impact = 0.99*it->second->overall_impact + 0.01*impact_diff;
				} else {
					it->second->overall_impact = (1.0 - 1.0/it->second->count)*it->second->overall_impact + 1.0/it->second->count*impact_diff;
				}
			}

			for (int a_index = 0; a_index < closest_diffs[d_index].second; a_index++) {
				int curr_action = closest_match->sequence[a_index];
				map<int, pair<int, double>>::iterator rel_it = it->second->rel_pre_impacts.find(curr_action);
				if (rel_it == it->second->rel_pre_impacts.end()) {
					it->second->rel_pre_impacts[curr_action] = {1, impact_diff};
				} else {
					rel_it->second.first++;
					if (rel_it->second.first >= 100) {
						rel_it->second.second = 0.99*rel_it->second.second + 0.01*impact_diff;
					} else {
						rel_it->second.second = (1.0 - 1.0/rel_it->second.first)*rel_it->second.second + 1.0/rel_it->second.first*impact_diff;
					}
				}
			}
			for (int a_index = closest_diffs[d_index].second + 1; a_index < (int)closest_match->sequence.size(); a_index++) {
				int curr_action = closest_match->sequence[a_index];
				map<int, pair<int, double>>::iterator rel_it = it->second->rel_post_impacts.find(curr_action);
				if (rel_it == it->second->rel_post_impacts.end()) {
					it->second->rel_post_impacts[curr_action] = {1, impact_diff};
				} else {
					rel_it->second.first++;
					if (rel_it->second.first >= 100) {
						rel_it->second.second = 0.99*rel_it->second.second + 0.01*impact_diff;
					} else {
						rel_it->second.second = (1.0 - 1.0/rel_it->second.first)*rel_it->second.second + 1.0/rel_it->second.first*impact_diff;
					}
				}
			}
		}
	}
}
