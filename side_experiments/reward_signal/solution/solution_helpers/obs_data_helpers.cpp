#include "solution_helpers.h"

#include <cmath>
#include <iostream>

#include "abstract_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"

using namespace std;

const int MIN_CONSIDER_SAMPLE_SIZE = 40;

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

void add_existing_hit_obs_data_helper(ScopeHistory* scope_history,
									  map<ObsNode*, ObsData>& obs_data) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				add_existing_hit_obs_data_helper(scope_node_history->scope_history,
												 obs_data);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				obs_data[obs_node].existing_hit_histories.push_back(obs_node_history->obs_history);
			}
			break;
		}
	}
}

void add_existing_miss_obs_data_helper(ScopeHistory* scope_history,
									   map<ObsNode*, ObsData>& obs_data) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				add_existing_miss_obs_data_helper(scope_node_history->scope_history,
												  obs_data);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				obs_data[obs_node].existing_miss_histories.push_back(obs_node_history->obs_history);
			}
			break;
		}
	}
}

void add_new_obs_data_helper(ScopeHistory* scope_history,
							 map<ObsNode*, ObsData>& obs_data) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		AbstractNode* node = it->second->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				add_new_obs_data_helper(scope_node_history->scope_history,
										obs_data);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* obs_node = (ObsNode*)node;
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				obs_data[obs_node].new_histories.push_back(obs_node_history->obs_history);
			}
			break;
		}
	}
}

bool compare_obs_data(map<ObsNode*, ObsData>& obs_data,
					  double hit_ratio,
					  int new_count,
					  SolutionWrapper* wrapper) {
	double new_factor = (double)MEASURE_ITERS * hit_ratio / (double)new_count;

	double sum_ratios = 0.0;
	int count = 0;
	for (map<ObsNode*, ObsData>::iterator it = obs_data.begin();
			it != obs_data.end(); it++) {
		double existing_count = (double)it->second.existing_miss_histories.size() + (double)it->second.existing_hit_histories.size();
		double new_count = (double)(it->second.existing_miss_histories.size() + it->second.existing_hit_histories.size());
		if (existing_count >= MIN_CONSIDER_SAMPLE_SIZE
				&& new_count >= MIN_CONSIDER_SAMPLE_SIZE) {
			for (int o_index = 0; o_index < wrapper->num_obs; o_index++) {
				double sum_existing_miss_vals = 0.0;
				for (int h_index = 0; h_index < (int)it->second.existing_miss_histories.size(); h_index++) {
					sum_existing_miss_vals += it->second.existing_miss_histories[h_index][o_index];
				}

				double sum_existing_vals = sum_existing_miss_vals;
				for (int h_index = 0; h_index < (int)it->second.existing_hit_histories.size(); h_index++) {
					sum_existing_vals += it->second.existing_hit_histories[h_index][o_index];
				}
				double existing_average = sum_existing_vals / existing_count;

				double sum_existing_variances = 0.0;
				for (int h_index = 0; h_index < (int)it->second.existing_miss_histories.size(); h_index++) {
					sum_existing_variances += (it->second.existing_miss_histories[h_index][o_index] - existing_average)
						* (it->second.existing_miss_histories[h_index][o_index] - existing_average);
				}
				for (int h_index = 0; h_index < (int)it->second.existing_hit_histories.size(); h_index++) {
					sum_existing_variances += (it->second.existing_hit_histories[h_index][o_index] - existing_average)
						* (it->second.existing_hit_histories[h_index][o_index] - existing_average);
				}
				double existing_standard_deviation = sqrt(sum_existing_variances / existing_count);
				if (existing_standard_deviation < MIN_STANDARD_DEVIATION) {
					existing_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				double sum_new_vals = sum_existing_miss_vals;
				for (int h_index = 0; h_index < (int)it->second.new_histories.size(); h_index++) {
					sum_new_vals += new_factor * it->second.new_histories[h_index][o_index];
				}
				double new_average = sum_new_vals / new_count;

				double sum_new_variances = 0.0;
				for (int h_index = 0; h_index < (int)it->second.existing_miss_histories.size(); h_index++) {
					sum_new_variances += (it->second.existing_miss_histories[h_index][o_index] - new_average)
						* (it->second.existing_miss_histories[h_index][o_index] - new_average);
				}
				for (int h_index = 0; h_index < (int)it->second.new_histories.size(); h_index++) {
					sum_new_variances += new_factor * (it->second.new_histories[h_index][o_index] - new_average)
						* (it->second.new_histories[h_index][o_index] - new_average);
				}
				double new_standard_deviation = sqrt(sum_new_variances / new_count);
				if (new_standard_deviation < MIN_STANDARD_DEVIATION) {
					new_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				sum_ratios += new_standard_deviation / existing_standard_deviation;
				count++;
			}
		}
	}

	double ratio = sum_ratios / (double)count;
	if (ratio > 1.0) {
		return false;
	} else {
		return true;
	}
}
