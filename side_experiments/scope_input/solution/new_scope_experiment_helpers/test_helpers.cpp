#include "new_scope_experiment.h"

#include <iostream>

#include "constants.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

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

void NewScopeExperiment::test_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		NewScopeExperimentHistory* history) {
	history->test_location_index = location_index;

	switch (this->test_location_states[location_index]) {
	case LOCATION_STATE_MEASURE_NEW:
	case LOCATION_STATE_VERIFY_NEW_1ST:
	case LOCATION_STATE_VERIFY_NEW_2ND:
		{
			ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);

			inner_scope_history->input_history = vector<double>(this->new_scope->num_inputs);
			for (int i_index = 0; i_index < this->new_scope->num_inputs; i_index++) {
				fetch_input(run_helper,
							scope_history,
							this->new_scope_inputs[i_index],
							inner_scope_history->input_history[i_index]);
			}

			this->new_scope->activate(problem,
									  run_helper,
									  inner_scope_history);
			delete inner_scope_history;

			curr_node = this->test_location_exits[location_index];
		}
		break;
	}
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_history;

	bool is_fail = false;

	switch (this->test_location_states[history->test_location_index]) {
	case LOCATION_STATE_MEASURE_EXISTING:
		this->test_location_existing_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_NUM_DATAPOINTS) {
			this->test_location_states[history->test_location_index] = LOCATION_STATE_MEASURE_NEW;
			this->test_location_counts[history->test_location_index] = 0;
		}

		break;
	case LOCATION_STATE_MEASURE_NEW:
		this->test_location_new_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_scores[history->test_location_index]
					> this->test_location_existing_scores[history->test_location_index]) {
			#endif /* MDEBUG */
				this->test_location_states[history->test_location_index] = LOCATION_STATE_VERIFY_EXISTING_1ST;
				this->test_location_existing_scores[history->test_location_index] = 0.0;
				this->test_location_new_scores[history->test_location_index] = 0.0;
				this->test_location_counts[history->test_location_index] = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_1ST:
		this->test_location_existing_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			this->test_location_states[history->test_location_index] = LOCATION_STATE_VERIFY_NEW_1ST;
			this->test_location_counts[history->test_location_index] = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_1ST:
		this->test_location_new_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_scores[history->test_location_index]
					> this->test_location_existing_scores[history->test_location_index]) {
			#endif /* MDEBUG */
				this->test_location_states[history->test_location_index] = LOCATION_STATE_VERIFY_EXISTING_2ND;
				this->test_location_existing_scores[history->test_location_index] = 0.0;
				this->test_location_new_scores[history->test_location_index] = 0.0;
				this->test_location_counts[history->test_location_index] = 0;
			} else {
				is_fail = true;
			}
		}

		break;
	case LOCATION_STATE_VERIFY_EXISTING_2ND:
		this->test_location_existing_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			this->test_location_states[history->test_location_index] = LOCATION_STATE_VERIFY_NEW_2ND;
			this->test_location_counts[history->test_location_index] = 0;
		}

		break;
	case LOCATION_STATE_VERIFY_NEW_2ND:
		this->test_location_new_scores[history->test_location_index] += target_val;
		this->test_location_counts[history->test_location_index]++;

		if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->test_location_new_scores[history->test_location_index]
					> this->test_location_existing_scores[history->test_location_index]) {
			#endif /* MDEBUG */
				ScopeNode* new_scope_node = new ScopeNode();
				new_scope_node->parent = this->scope_context;
				new_scope_node->id = this->scope_context->node_counter;
				this->scope_context->node_counter++;

				new_scope_node->scope = this->new_scope;

				if (this->test_location_exits[history->test_location_index] == NULL) {
					new_scope_node->next_node_id = -1;
					new_scope_node->next_node = NULL;
				} else {
					new_scope_node->next_node_id = this->test_location_exits[history->test_location_index]->id;
					new_scope_node->next_node = this->test_location_exits[history->test_location_index];
				}

				this->successful_location_starts.push_back(this->test_location_starts[history->test_location_index]);
				this->successful_location_is_branch.push_back(this->test_location_is_branch[history->test_location_index]);
				this->successful_scope_nodes.push_back(new_scope_node);

				this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
				this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
				this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
				this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
				this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
				this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
				this->test_location_counts.erase(this->test_location_counts.begin() + history->test_location_index);

				this->generalize_iter++;
			} else {
				is_fail = true;
			}
		}

		break;
	}

	if (is_fail) {
		this->test_location_starts[history->test_location_index]->experiment = NULL;
		this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
		this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
		this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
		this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
		this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
		this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
		this->test_location_counts.erase(this->test_location_counts.begin() + history->test_location_index);

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
		#if defined(MDEBUG) && MDEBUG
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			this->test_location_starts[t_index]->experiment = NULL;
		}
		this->test_location_starts.clear();
		this->test_location_is_branch.clear();
		this->test_location_exits.clear();
		this->test_location_states.clear();
		this->test_location_existing_scores.clear();
		this->test_location_new_scores.clear();
		this->test_location_counts.clear();

		this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
		this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);
		this->verify_can_random = vector<bool>(NUM_VERIFY_SAMPLES);

		this->state = NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY;
		this->state_iter = 0;
		#else
		this->result = EXPERIMENT_RESULT_SUCCESS;
		#endif /* MDEBUG */
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
