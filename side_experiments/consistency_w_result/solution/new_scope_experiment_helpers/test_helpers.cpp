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

// const double MATCH_SCORE = -0.01;
const double MATCH_SCORE = -0.003;

void NewScopeExperiment::test_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		NewScopeExperimentHistory* history) {
	history->hit_test = true;

	run_helper.check_match = true;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->experiment_activate(problem,
										 run_helper,
										 inner_scope_history);
	delete inner_scope_history;

	curr_node = this->test_location_exit;
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	bool is_fail = false;

	double sum_factors = 0.0;
	for (int f_index = 0; f_index < (int)run_helper.match_factors.size(); f_index++) {
		sum_factors += run_helper.match_factors[f_index];
	}
	double average_factor = sum_factors / (int)run_helper.match_factors.size();
	this->match_histories.push_back(average_factor);

	switch (this->test_location_state) {
	case LOCATION_STATE_MEASURE:
		this->test_location_score += target_val - run_helper.result;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_NUM_DATAPOINTS) {
			double sum_matches = 0.0;
			for (int h_index = 0; h_index < (int)this->match_histories.size(); h_index++) {
				sum_matches += this->match_histories[h_index];
			}
			double average_match = sum_matches / (int)this->match_histories.size();
			this->match_histories.clear();
			cout << "average_match: " << average_match << endl;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_score > MATCH_SCORE) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_1ST;
				this->test_location_score = 0.0;
				this->test_location_count = 0;

				this->test_match_histories.clear();
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_1ST:
		this->test_location_score += target_val - run_helper.result;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			double sum_matches = 0.0;
			for (int h_index = 0; h_index < (int)this->match_histories.size(); h_index++) {
				sum_matches += this->match_histories[h_index];
			}
			double average_match = sum_matches / (int)this->match_histories.size();
			this->match_histories.clear();
			cout << "average_match: " << average_match << endl;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_score > MATCH_SCORE) {
			#endif /* MDEBUG */
				this->test_location_state = LOCATION_STATE_VERIFY_2ND;
				this->test_location_score = 0.0;
				this->test_location_count = 0;

				this->test_match_histories.clear();
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_2ND:
		this->test_location_score += target_val - run_helper.result;
		this->test_location_count++;

		if (this->test_location_count >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			double sum_matches = 0.0;
			for (int h_index = 0; h_index < (int)this->match_histories.size(); h_index++) {
				sum_matches += this->match_histories[h_index];
			}
			double average_match = sum_matches / (int)this->match_histories.size();
			this->match_histories.clear();
			cout << "average_match: " << average_match << endl;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_score > MATCH_SCORE) {
			#endif /* MDEBUG */
				double new_score = this->test_location_score / NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS;
				// this->improvement += new_score;
				this->improvement = new_score;

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

				new_scope_node->average_instances_per_run = this->test_location_start->average_instances_per_run;

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

		#if defined(MDEBUG) && MDEBUG
		this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
		this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

		this->state = NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY;
		this->state_iter = 0;
		#else
		this->result = EXPERIMENT_RESULT_SUCCESS;
		#endif /* MDEBUG */
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
