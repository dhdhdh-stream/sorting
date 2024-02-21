#include "seed_experiment.h"

using namespace std;

void SeedExperiment::measure_filter_backprop(double target_val,
											 SeedExperimentOverallHistory* history) {
	if (history->has_target) {
		if (target_val > this->existing_average_score + this->existing_score_standard_deviation) {
			this->i_is_higher_histories.push_back(true);
		} else {
			this->i_is_higher_histories.push_back(false);
		}

		if ((int)this->i_target_val_histories.size() >= solution->curr_num_datapoints) {
			int num_seed = 0;
			int num_higher = 0;
			for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
				if (this->i_is_seed_histories[d_index]) {
					if (this->i_is_higher_histories[d_index]) {
						num_higher++;
					}
					num_seed++;
				}
			}
			double higher_ratio = (double)num_higher / (double)num_seed;

			double improve_threshold;
			if (this->curr_higher_ratio > 0.5) {
				improve_threshold = 0.1 * (1.0 - this->curr_higher_ratio);
			} else {
				improve_threshold = 0.1 * this->curr_higher_ratio;
			}

			if (higher_ratio > this->curr_higher_ratio + improve_threshold) {
				this->curr_higher_ratio = higher_ratio;

				if (this->curr_gather != NULL) {
					this->curr_gather->add_to_scope();
					this->gathers.push_back(this->curr_gather);
					this->curr_gather = NULL;

					this->train_gather_iter = 0;
				}

				this->curr_filter_is_success = true;

				this->state = SEED_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				if (this->curr_gather != NULL) {
					this->curr_gather->clean_fail();
					delete this->curr_gather;
					this->curr_gather = NULL;
				}

				this->train_gather_iter++;
				if (this->gather_iter >= TRAIN_GATHER_ITER_LIMIT) {
					if (this->curr_filter_is_success) {
						this->curr_filter->add_to_scope();
						this->filters.push_back(this->curr_filter);
					} else {
						this->curr_filter->clean_fail();
						delete this->curr_filter;
					}
					this->curr_filter = NULL;

					if (this->filter_step_index == (int)this->best_step_types.size()) {
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						this->state = SEED_EXPERIMENT_STATE_FIND_FILTER;
						this->state_iter = 0;
						create_filter();
						this->sub_state_iter = 0;
					}
				} else {
					this->curr_gather_is_higher = 0;
					this->curr_gather_score = 0.0;

					this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
					this->state_iter = 0;
					this->sub_state_iter = -1;
				}
			}
		}
	}
}
