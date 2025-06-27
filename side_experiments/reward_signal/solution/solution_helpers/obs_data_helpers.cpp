#include "solution_helpers.h"

#include <cmath>

#include "abstract_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

const int OBS_DATA_MIN_DATAPOINTS = 40;

const double STANDARD_DEVIATION_MAX_DIFF = 1.3;

const double MAX_MISMATCH_RATIO = 0.1;

bool has_match_helper(ScopeHistory* scope_history,
					  AbstractNode* explore_node,
					  bool is_branch) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		if (it->second->node == explore_node) {
			if (explore_node->type == NODE_TYPE_BRANCH) {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
				if (branch_node_history->is_branch == is_branch) {
					return true;
				}
			} else {
				return true;
			}
		}

		if (it->second->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
			bool has_match = has_match_helper(scope_node_history->scope_history,
											  explore_node,
											  is_branch);
			if (has_match) {
				return true;
			}
		}
	}

	return false;
}

void add_obs_data_helper(ScopeHistory* scope_history,
						 map<ObsNode*, ObsData>& obs_data) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				add_obs_data_helper(scope_node_history->scope_history,
									obs_data);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				obs_data[obs_node].val_histories.push_back(obs_node_history->obs_history);
			}
			break;
		}
	}
}

void process_obs_data(map<ObsNode*, ObsData>& obs_data) {
	map<ObsNode*, ObsData>::iterator it = obs_data.begin();
	while (it != obs_data.end()) {
		if (it->second.val_histories.size() >= OBS_DATA_MIN_DATAPOINTS) {
			int obs_size = (int)it->second.val_histories[0].size();

			it->second.averages = vector<double>(obs_size);
			it->second.standard_deviations = vector<double>(obs_size);

			for (int o_index = 0; o_index < obs_size; o_index++) {
				double sum_vals = 0.0;
				for (int h_index = 0; h_index < (int)it->second.val_histories.size(); h_index++) {
					sum_vals += it->second.val_histories[h_index][o_index];
				}
				it->second.averages[o_index] = sum_vals / (double)it->second.val_histories.size();

				double sum_variances = 0.0;
				for (int h_index = 0; h_index < (int)it->second.val_histories.size(); h_index++) {
					sum_variances += (it->second.val_histories[h_index][o_index] - it->second.averages[o_index])
						* (it->second.val_histories[h_index][o_index] - it->second.averages[o_index]);
				}
				it->second.standard_deviations[o_index] = sqrt(sum_variances / (double)it->second.val_histories.size());
				if (it->second.standard_deviations[o_index] < MIN_STANDARD_DEVIATION) {
					it->second.standard_deviations[o_index] = MIN_STANDARD_DEVIATION;
				}
			}

			it++;
		} else {
			it = obs_data.erase(it);
		}
	}
}

bool compare_obs_data(map<ObsNode*, ObsData>& existing_obs_data,
					  map<ObsNode*, ObsData>& new_obs_data) {
	int num_compare = 0;
	int num_mismatch = 0;

	for (map<ObsNode*, ObsData>::iterator new_it = new_obs_data.begin();
			new_it != new_obs_data.end(); new_it++) {
		map<ObsNode*, ObsData>::iterator existing_it = existing_obs_data.find(new_it->first);
		if (existing_it != existing_obs_data.end()) {
			bool has_mismatch = false;
			for (int o_index = 0; o_index < (int)new_it->second.standard_deviations.size(); o_index++) {
				if (existing_it->second.standard_deviations[o_index] > STANDARD_DEVIATION_MAX_DIFF * new_it->second.standard_deviations[o_index]
						|| new_it->second.standard_deviations[o_index] > STANDARD_DEVIATION_MAX_DIFF * existing_it->second.standard_deviations[o_index]) {
					has_mismatch = true;
					break;
				} else {
					double existing_scaled_std = existing_it->second.standard_deviations[o_index]
						/ (double)existing_it->second.val_histories.size();
					double new_scaled_std = new_it->second.standard_deviations[o_index]
						/ (double)new_it->second.val_histories.size();
					double denom = sqrt(existing_scaled_std * existing_scaled_std + new_scaled_std * new_scaled_std);
					double avg_diff = existing_it->second.averages[o_index] - new_it->second.averages[o_index];
					double t_score = abs(avg_diff / denom);
					if (t_score > 2.326) {
						has_mismatch = true;
						break;
					}
				}
			}

			num_compare++;
			if (has_mismatch) {
				num_mismatch++;
			}
		}
	}

	double mismatch_ratio = (double)num_mismatch / (double)num_compare;
	if (mismatch_ratio > MAX_MISMATCH_RATIO) {
		return false;
	} else {
		return true;
	}
}
