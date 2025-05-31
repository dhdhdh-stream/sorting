#include "solution_helpers.h"

#include <cmath>
#include <iostream>

#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void sum_obs_average_helpers(ScopeHistory* scope_history) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				sum_obs_average_helpers(scope_node_history->scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				ObsNode* obs_node = (ObsNode*)it->second->node;

				obs_node->sum_obs_vals += obs_node_history->obs_history[0];
				obs_node->obs_count++;
			}
			break;
		}
	}
}

void sum_obs_variance_helpers(ScopeHistory* scope_history) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				sum_obs_variance_helpers(scope_node_history->scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				ObsNode* obs_node = (ObsNode*)it->second->node;

				obs_node->sum_obs_variances += (obs_node_history->obs_history[0] - obs_node->average_val)
					* (obs_node_history->obs_history[0] - obs_node->average_val);
			}
			break;
		}
	}
}

void gather_match_datapoints_helpers(ScopeHistory* scope_history) {
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		switch (it->second->node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
				gather_match_datapoints_helpers(scope_node_history->scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNodeHistory* obs_node_history = (ObsNodeHistory*)it->second;
				ObsNode* obs_node = (ObsNode*)it->second->node;
				obs_node->gather_match_datapoints(obs_node_history,
												  scope_history);
			}
			break;
		}
	}
}

void update_matches(vector<ScopeHistory*>& scope_histories) {
	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		sum_obs_average_helpers(scope_histories[h_index]);
	}

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = solution->scopes[s_index]->nodes.begin();
				it != solution->scopes[s_index]->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (!obs_node->is_init) {
					if (obs_node->obs_count > 0) {
						obs_node->average_val = obs_node->sum_obs_vals / obs_node->obs_count;

						// temp
						obs_node->min_average_val = obs_node->average_val;
						obs_node->max_average_val = obs_node->average_val;
					}
				} else {
					if (obs_node->obs_count > MEASURE_ITERS) {
						obs_node->average_val = obs_node->sum_obs_vals / obs_node->obs_count;
					} else {
						if (obs_node->obs_count > 0) {
							double new_average_val = obs_node->sum_obs_vals / obs_node->obs_count;
							double ratio = (double)obs_node->obs_count / (double)MEASURE_ITERS;
							obs_node->average_val = (1.0 - ratio) * obs_node->average_val + ratio * new_average_val;
						}
					}
				}
			}
		}
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		sum_obs_variance_helpers(scope_histories[h_index]);
	}

	double overall_standard_deviation = sqrt(solution->obs_variances[0]);
	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = solution->scopes[s_index]->nodes.begin();
				it != solution->scopes[s_index]->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (!obs_node->is_init) {
					if (obs_node->obs_count > 0) {
						obs_node->average_variance = obs_node->sum_obs_variances / obs_node->obs_count;
						obs_node->standard_deviation = sqrt(obs_node->average_variance);
						if (obs_node->standard_deviation < MIN_STANDARD_DEVIATION) {
							obs_node->standard_deviation = MIN_STANDARD_DEVIATION;
						}
					}

					if (obs_node->standard_deviation > MAX_OVERALL_OBS_FACTOR * overall_standard_deviation) {
						obs_node->check_consistency = false;
					} else {
						obs_node->check_consistency = true;
					}

					// temp
					obs_node->min_standard_deviation = obs_node->standard_deviation;
					obs_node->max_standard_deviation = obs_node->standard_deviation;
				} else {
					if (obs_node->obs_count > MEASURE_ITERS) {
						obs_node->average_variance = obs_node->sum_obs_variances / obs_node->obs_count;
						obs_node->standard_deviation = sqrt(obs_node->average_variance);
						if (obs_node->standard_deviation < MIN_STANDARD_DEVIATION) {
							obs_node->standard_deviation = MIN_STANDARD_DEVIATION;
						}

						if (obs_node->standard_deviation > MAX_OVERALL_OBS_FACTOR * overall_standard_deviation) {
							obs_node->check_consistency = false;
						} else {
							obs_node->check_consistency = true;
						}

						// temp
						if (obs_node->standard_deviation < obs_node->min_standard_deviation) {
							obs_node->min_standard_deviation = obs_node->standard_deviation;
							obs_node->min_average_val = obs_node->average_val;
						}
						if (obs_node->standard_deviation > obs_node->max_standard_deviation) {
							obs_node->max_standard_deviation = obs_node->standard_deviation;
							obs_node->max_average_val = obs_node->average_val;
						}
					} else {
						if (obs_node->obs_count > 0) {
							double new_average_variance = obs_node->sum_obs_variances / obs_node->obs_count;
							double ratio = (double)obs_node->obs_count / (double)MEASURE_ITERS;
							obs_node->average_variance = (1.0 - ratio) * obs_node->average_variance + ratio * new_average_variance;
							obs_node->standard_deviation = sqrt(obs_node->average_variance);
							if (obs_node->standard_deviation < MIN_STANDARD_DEVIATION) {
								obs_node->standard_deviation = MIN_STANDARD_DEVIATION;
							}

							if (obs_node->standard_deviation > MAX_OVERALL_OBS_FACTOR * overall_standard_deviation) {
								obs_node->check_consistency = false;
							} else {
								obs_node->check_consistency = true;
							}

							// temp
							if (obs_node->standard_deviation < obs_node->min_standard_deviation) {
								obs_node->min_standard_deviation = obs_node->standard_deviation;
								obs_node->min_average_val = obs_node->average_val;
							}
							if (obs_node->standard_deviation > obs_node->max_standard_deviation) {
								obs_node->max_standard_deviation = obs_node->standard_deviation;
								obs_node->max_average_val = obs_node->average_val;
							}
						}
					}
				}
			}
		}
	}

	for (int h_index = 0; h_index < (int)scope_histories.size(); h_index++) {
		gather_match_datapoints_helpers(scope_histories[h_index]);
	}

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = solution->scopes[s_index]->nodes.begin();
				it != solution->scopes[s_index]->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				obs_node->update_matches();
			}
		}
	}

	for (int s_index = 0; s_index < (int)solution->scopes.size(); s_index++) {
		for (map<int, AbstractNode*>::iterator it = solution->scopes[s_index]->nodes.begin();
				it != solution->scopes[s_index]->nodes.end(); it++) {
			it->second->is_init = true;
		}
	}
}
