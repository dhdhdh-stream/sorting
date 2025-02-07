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
const int NEW_SCOPE_NUM_DATAPOINTS = 200;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 1000;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void NewScopeExperiment::test_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	history->test_location_index = location_index;

	switch (this->test_location_states[location_index]) {
	case LOCATION_STATE_MEASURE_NEW:
	case LOCATION_STATE_VERIFY_NEW_1ST:
	case LOCATION_STATE_VERIFY_NEW_2ND:
		{
			ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
			this->new_scope->experiment_activate(problem,
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
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					int experiment_index;
					for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
						if (this->test_location_starts[t_index]->experiments[e_index] == this) {
							experiment_index = e_index;
							break;
						}
					}
					this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
				}
				this->test_location_starts.clear();
				this->test_location_is_branch.clear();
				this->test_location_exits.clear();
				this->test_location_states.clear();
				this->test_location_existing_scores.clear();
				this->test_location_new_scores.clear();
				this->test_location_counts.clear();

				this->generalize_iter++;
			} else {
				is_fail = true;
			}
		}

		break;
	}

	if (is_fail) {
		int experiment_index;
		for (int e_index = 0; e_index < (int)this->test_location_starts[history->test_location_index]->experiments.size(); e_index++) {
			if (this->test_location_starts[history->test_location_index]->experiments[e_index] == this) {
				experiment_index = e_index;
				break;
			}
		}
		this->test_location_starts[history->test_location_index]->experiments.erase(
			this->test_location_starts[history->test_location_index]->experiments.begin() + experiment_index);
		/**
		 * - can simply remove first
		 */

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
			int experiment_index;
			for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
				if (this->test_location_starts[t_index]->experiments[e_index] == this) {
					experiment_index = e_index;
					break;
				}
			}
			this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
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

		this->state = NEW_SCOPE_EXPERIMENT_STATE_CAPTURE_VERIFY;
		this->state_iter = 0;
		#else
		this->result = EXPERIMENT_RESULT_SUCCESS;
		#endif /* MDEBUG */
	} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}
