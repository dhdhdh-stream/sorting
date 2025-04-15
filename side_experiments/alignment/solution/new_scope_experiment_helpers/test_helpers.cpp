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

void NewScopeExperiment::test_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->experiment_activate(problem,
										 run_helper,
										 inner_scope_history);
	delete inner_scope_history;

	curr_node = this->exit_next_node;
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	this->sum_score += target_val;

	this->state_iter++;
	switch (this->state) {
	case NEW_SCOPE_EXPERIMENT_STATE_MEASURE:
		if (this->state_iter >= NEW_SCOPE_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double new_score = this->sum_score / this->state_iter;
			if (new_score > this->node_context->average_score) {
			#endif /* MDEBUG */
				this->state = NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST;
				this->state_iter = 0;
				this->sum_score = 0.0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_1ST:
		if (this->state_iter >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double new_score = this->sum_score / this->state_iter;
			if (new_score > this->node_context->average_score) {
			#endif /* MDEBUG */
				this->state = NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND;
				this->state_iter = 0;
				this->sum_score = 0.0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
		break;
	case NEW_SCOPE_EXPERIMENT_STATE_VERIFY_2ND:
		if (this->state_iter >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			double new_score = this->sum_score / this->state_iter;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (new_score > this->node_context->average_score) {
			#endif /* MDEBUG */
				this->improvement = new_score - this->node_context->average_score;

				cout << "NewScopeExperiment success" << endl;
				cout << "this->improvement: " << this->improvement << endl;

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
