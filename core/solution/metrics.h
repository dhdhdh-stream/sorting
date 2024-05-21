#ifndef METRICS_H
#define METRICS_H

#include <vector>

class Metrics {
public:
	int curr_explore_id;
	int curr_explore_type;

	double curr_sum_timestamp_score;
	int curr_num_instances;

	int next_explore_id;
	int next_explore_type;

	int next_num_instances;
	int next_max_num_actions;
	double next_local_sum_num_actions;

	Metrics(int curr_explore_id,
			int curr_explore_type,
			int next_explore_id,
			int next_explore_type) {
		this->curr_explore_id = curr_explore_id;
		this->curr_explore_type = curr_explore_type;

		this->curr_sum_timestamp_score = 0.0;
		this->curr_num_instances = 0;

		this->next_explore_id = next_explore_id;
		this->next_explore_type = next_explore_type;

		this->next_num_instances = 0;
		this->next_max_num_actions = 0;
		this->next_local_sum_num_actions = 0.0;
	}
};

#endif /* METRICS_H */