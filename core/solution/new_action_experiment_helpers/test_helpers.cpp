/**
 * - don't use eval
 *   - becomes increasingly inaccurate with more successes
 *     - especially with chaining
 * 
 * - TODO: or at least try not using local eval
 *   - perhaps exclude local eval in general
 */

#include "new_action_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_ACTION_NUM_DATAPOINTS = 10;
const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 10;
const int NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_ACTION_NUM_DATAPOINTS = 40;
const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 200;
const int NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS = 1000;
#endif /* MDEBUG */

void NewActionExperiment::test_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewActionExperimentHistory* history) {
	history->test_location_index = location_index;

	switch (this->test_location_states[location_index]) {
	case NEW_ACTION_EXPERIMENT_MEASURE_NEW:
	case NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW:
	case NEW_ACTION_EXPERIMENT_VERIFY_2ND_NEW:
		this->new_scope->activate(problem,
								  context,
								  run_helper);

		curr_node = this->test_location_exits[location_index];
	}
}

void NewActionExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories.back();

	bool is_fail = false;

	if (run_helper.exceeded_limit) {
		is_fail = true;
	} else {
		switch (this->test_location_states[history->test_location_index]) {
		case NEW_ACTION_EXPERIMENT_MEASURE_EXISTING:
			{
				this->test_location_existing_scores[history->test_location_index] += target_val;
				this->test_location_existing_counts[history->test_location_index]++;

				if (this->test_location_existing_counts[history->test_location_index] >= NEW_ACTION_NUM_DATAPOINTS) {
					this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_MEASURE_NEW;
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_MEASURE_NEW:
			{
				this->test_location_new_scores[history->test_location_index] += target_val;
				this->test_location_new_counts[history->test_location_index]++;

				if (this->test_location_new_counts[history->test_location_index] >= NEW_ACTION_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double existing_score = this->test_location_existing_scores[history->test_location_index]
						/ this->test_location_existing_counts[history->test_location_index];
					double new_score = this->test_location_new_scores[history->test_location_index]
						/ this->test_location_new_counts[history->test_location_index];

					if (new_score >= existing_score) {
					#endif /* MDEBUG */
						this->test_location_existing_scores[history->test_location_index] = 0.0;
						this->test_location_existing_counts[history->test_location_index] = 0;
						this->test_location_new_scores[history->test_location_index] = 0.0;
						this->test_location_new_counts[history->test_location_index] = 0;
						this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_1ST_EXISTING;
					} else {
						is_fail = true;
					}
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_1ST_EXISTING:
			{
				this->test_location_existing_scores[history->test_location_index] += target_val;
				this->test_location_existing_counts[history->test_location_index]++;

				if (this->test_location_existing_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS) {
					this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW;
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW:
			{
				this->test_location_new_scores[history->test_location_index] += target_val;
				this->test_location_new_counts[history->test_location_index]++;

				if (this->test_location_new_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double existing_score = this->test_location_existing_scores[history->test_location_index]
						/ this->test_location_existing_counts[history->test_location_index];
					double new_score = this->test_location_new_scores[history->test_location_index]
						/ this->test_location_new_counts[history->test_location_index];

					if (new_score >= existing_score) {
					#endif /* MDEBUG */
						this->test_location_existing_scores[history->test_location_index] = 0.0;
						this->test_location_existing_counts[history->test_location_index] = 0;
						this->test_location_new_scores[history->test_location_index] = 0.0;
						this->test_location_new_counts[history->test_location_index] = 0;
						this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_2ND_EXISTING;
					} else {
						is_fail = true;
					}
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_2ND_EXISTING:
			{
				this->test_location_existing_scores[history->test_location_index] += target_val;
				this->test_location_existing_counts[history->test_location_index]++;

				if (this->test_location_existing_counts[history->test_location_index] >= NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS) {
					this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_2ND_NEW;
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_2ND_NEW:
			{
				this->test_location_new_scores[history->test_location_index] += target_val;
				this->test_location_new_counts[history->test_location_index]++;

				if (this->test_location_new_counts[history->test_location_index] >= NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					double existing_score = this->test_location_existing_scores[history->test_location_index]
						/ this->test_location_existing_counts[history->test_location_index];
					double new_score = this->test_location_new_scores[history->test_location_index]
						/ this->test_location_new_counts[history->test_location_index];

					if (new_score >= existing_score) {
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
						this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
						this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
						this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

						if (this->generalize_iter == -1
								&& this->successful_location_starts.size() == 0) {
							this->result = EXPERIMENT_RESULT_FAIL;
							/**
							 * - only continue if first succeeds
							 */
						} else {
							this->generalize_iter++;
						}
					} else {
						is_fail = true;
					}
				}
			}

			break;
		}
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
		this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
		this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
		this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

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
}
