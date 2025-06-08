#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 5;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 40;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 400;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void NewScopeExperiment::test_backprop(
		double target_val,
		NewScopeExperimentHistory* history) {
	bool is_fail = false;

	switch (this->test_location_state) {
	case LOCATION_STATE_MEASURE_EXISTING:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_MEASURE_NEW;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_MEASURE_NEW:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_EXISTING_1ST;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score = 0.0;
				this->test_location_count = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_1ST:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_VERIFY_NEW_1ST;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_1ST:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_EXISTING_2ND;
				this->test_location_existing_score = 0.0;
				this->test_location_new_score = 0.0;
				this->test_location_count = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_2ND:
		this->test_location_existing_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			this->test_location_state = LOCATION_STATE_VERIFY_NEW_2ND;
			this->test_location_count = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_2ND:
		this->test_location_new_score += target_val;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_score > this->test_location_existing_score) {
			#endif /* MDEBUG */
				double new_score = this->test_location_new_score / NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS;
				double existing_score = this->test_location_existing_score / NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS;
				this->improvement += (new_score - existing_score);

				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = this->scope_context;
				new_scope_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				new_scope_node->scope = this->new_scope;

				if (this->test_location_exit == NULL) {
					new_scope_node->next_node_id = -1;
					new_scope_node->next_node = NULL;
				} else {
					new_scope_node->next_node_id = this->test_location_exit->id;
					new_scope_node->next_node = this->test_location_exit;
				}

				this->successful_location_starts.push_back(this->test_location_start);
				this->successful_location_is_branch.push_back(this->test_location_is_branch);
				this->successful_scope_nodes.push_back(new_scope_node);

				this->test_location_start = NULL;

				this->generalize_iter++;
			} else {
				is_fail = true;
			}
		}

		break;
	}

	if (is_fail) {
		this->test_location_start->experiment = NULL;
		this->test_location_start = NULL;

		if (this->generalize_iter == -1
				&& this->successful_location_starts.size() == 0) {
			this->result = EXPERIMENT_RESULT_FAIL;
			/**
			 * - only continue if first succeeds
			 */
		} else {
			this->generalize_iter++;
		}
	}

	if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
		cout << "NewScopeExperiment success" << endl;
		cout << "this->improvement: " << this->improvement << endl;

		this->result = EXPERIMENT_RESULT_SUCCESS;
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
