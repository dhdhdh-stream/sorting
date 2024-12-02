#include "new_scope_experiment.h"

#include <iostream>

#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 5;
const int NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 2;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 100;
const int NEW_SCOPE_TRUTH_NUM_DATAPOINTS = 20;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 500;
const int NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 100;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 2000;
const int NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 400;
#endif /* MDEBUG */

void NewScopeExperiment::test_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	history->instance_count++;

	context.back().node_id = -1;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->new_scope_activate(problem,
										context,
										run_helper,
										inner_scope_history);
	delete inner_scope_history;

	curr_node = this->exit_next_node;
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		double final_score = (target_val - run_helper.result) / history->instance_count;
		this->score += final_score;
		this->sub_state_iter++;
	}
	this->state_iter++;

	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
		if (this->sub_state_iter >= NEW_SCOPE_NUM_DATAPOINTS
				&& this->state_iter >= NEW_SCOPE_TRUTH_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->score > 0.0) {
			#endif /* MDEBUG */
				this->score = 0.0;
				this->sub_state_iter = 0;
				this->state_iter = 0;
				this->state = NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}

		break;
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
		if (this->sub_state_iter >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS
				&& this->state_iter >= NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->score > 0.0) {
			#endif /* MDEBUG */
				this->score = 0.0;
				this->sub_state_iter = 0;
				this->state_iter = 0;
				this->state = NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}

		break;
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
		if (this->sub_state_iter >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS
				&& this->state_iter >= NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->score > 0.0) {
			#endif /* MDEBUG */
				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}

		break;
	}
}
