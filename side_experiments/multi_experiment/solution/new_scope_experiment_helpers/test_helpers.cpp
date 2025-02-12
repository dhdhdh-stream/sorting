#include "new_scope_experiment.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 100;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 500;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

void NewScopeExperiment::test_backprop(NewScopeExperimentHistory* history,
									   double target_val) {
	switch (this->test_location_states[history->test_location_index]) {
	case LOCATION_STATE_MEASURE_EXISTING:
	case LOCATION_STATE_VERIFY_EXISTING_1ST:
	case LOCATION_STATE_VERIFY_EXISTING_2ND:
		this->test_location_existing_scores[history->test_location_index] += target_val;
		this->test_location_existing_counts[history->test_location_index]++;
		break;
	case LOCATION_STATE_MEASURE_NEW:
	case LOCATION_STATE_VERIFY_NEW_1ST:
	case LOCATION_STATE_VERIFY_NEW_2ND:
		this->test_location_new_scores[history->test_location_index] += target_val;
		this->test_location_new_counts[history->test_location_index]++;
		break;
	}
}

void NewScopeExperiment::test_update() {
	for (int t_index = (int)this->test_location_starts.size()-1; t_index >= 0; t_index--) {
		bool is_fail = false;

		switch (this->test_location_states[t_index]) {
		case LOCATION_STATE_MEASURE_EXISTING:
			if (this->test_location_existing_counts[t_index] >= NEW_SCOPE_NUM_DATAPOINTS) {
				this->test_location_states[t_index] = LOCATION_STATE_MEASURE_NEW;
			}
			break;
		case LOCATION_STATE_MEASURE_NEW:
			if (this->test_location_new_counts[t_index] >= NEW_SCOPE_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double existing_score = this->test_location_existing_scores[t_index]
					/ this->test_location_existing_counts[t_index];
				double new_score = this->test_location_new_scores[t_index]
					/ this->test_location_new_counts[t_index];
				if (new_score > existing_score) {
				#endif /* MDEBUG */
					this->test_location_states[t_index] = LOCATION_STATE_VERIFY_EXISTING_1ST;
					this->test_location_existing_scores[t_index] = 0.0;
					this->test_location_existing_counts[t_index] = 0;
					this->test_location_new_scores[t_index] = 0.0;
					this->test_location_new_counts[t_index] = 0;
				} else {
					is_fail = true;
				}
			}
			break;
		case LOCATION_STATE_VERIFY_EXISTING_1ST:
			if (this->test_location_existing_counts[t_index] >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
				this->test_location_states[t_index] = LOCATION_STATE_VERIFY_NEW_1ST;
			}
			break;
		case LOCATION_STATE_VERIFY_NEW_1ST:
			if (this->test_location_new_counts[t_index] >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double existing_score = this->test_location_existing_scores[t_index]
					/ this->test_location_existing_counts[t_index];
				double new_score = this->test_location_new_scores[t_index]
					/ this->test_location_new_counts[t_index];
				if (new_score > existing_score) {
				#endif /* MDEBUG */
					this->test_location_states[t_index] = LOCATION_STATE_VERIFY_EXISTING_2ND;
					this->test_location_existing_scores[t_index] = 0.0;
					this->test_location_existing_counts[t_index] = 0;
					this->test_location_new_scores[t_index] = 0.0;
					this->test_location_new_counts[t_index] = 0;
				} else {
					is_fail = true;
				}
			}
			break;
		case LOCATION_STATE_VERIFY_EXISTING_2ND:
			if (this->test_location_existing_counts[t_index] >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
				this->test_location_states[t_index] = LOCATION_STATE_VERIFY_NEW_2ND;
			}
			break;
		case LOCATION_STATE_VERIFY_NEW_2ND:
			if (this->test_location_new_counts[t_index] >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				double existing_score = this->test_location_existing_scores[t_index]
					/ this->test_location_existing_counts[t_index];
				double new_score = this->test_location_new_scores[t_index]
					/ this->test_location_new_counts[t_index];
				if (new_score > existing_score) {
				#endif /* MDEBUG */
					this->successful_location_starts.push_back(this->test_location_starts[t_index]);
					this->successful_location_is_branch.push_back(this->test_location_is_branch[t_index]);
					this->successful_location_exits.push_back(this->test_location_exits[t_index]);

					this->test_location_starts.erase(this->test_location_starts.begin() + t_index);
					this->test_location_is_branch.erase(this->test_location_is_branch.begin() + t_index);
					this->test_location_exits.erase(this->test_location_exits.begin() + t_index);
					this->test_location_states.erase(this->test_location_states.begin() + t_index);
					this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + t_index);
					this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + t_index);
					this->test_location_new_scores.erase(this->test_location_new_scores.begin() + t_index);
					this->test_location_new_counts.erase(this->test_location_new_counts.begin() + t_index);

					this->generalize_iter++;
				} else {
					is_fail = true;
				}
			}

			break;
		}

		if (is_fail) {
			this->test_location_starts[t_index]->experiment = NULL;

			this->test_location_starts.erase(this->test_location_starts.begin() + t_index);
			this->test_location_is_branch.erase(this->test_location_is_branch.begin() + t_index);
			this->test_location_exits.erase(this->test_location_exits.begin() + t_index);
			this->test_location_states.erase(this->test_location_states.begin() + t_index);
			this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + t_index);
			this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + t_index);
			this->test_location_new_scores.erase(this->test_location_new_scores.begin() + t_index);
			this->test_location_new_counts.erase(this->test_location_new_counts.begin() + t_index);

			if (this->generalize_iter == -1) {
				this->result = EXPERIMENT_RESULT_FAIL;
				/**
				 * - only continue if first succeeds
				 */
			} else {
				this->generalize_iter++;
			}
		}
	}

	if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
