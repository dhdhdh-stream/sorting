#include "new_action_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_ACTION_NUM_DATAPOINTS = 10;
const int NEW_ACTION_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 10;
const int NEW_ACTION_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS = 10;
const int NEW_ACTION_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 2;
#else
const int NEW_ACTION_NUM_DATAPOINTS = 50;
const int NEW_ACTION_TRUTH_NUM_DATAPOINTS = 20;
const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 250;
const int NEW_ACTION_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 100;
const int NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS = 1000;
const int NEW_ACTION_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 400;
#endif /* MDEBUG */

void NewActionExperiment::test_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewActionExperimentHistory* history) {
	if (history->test_location_index == -1) {
		history->test_location_index = location_index;
		history->instance_count = 0;
	}

	history->instance_count++;

	context.back().node = this->test_scope_nodes[location_index];

	this->new_scope->experiment_activate(problem,
										 context,
										 run_helper);

	context.back().node = NULL;

	curr_node = this->test_location_exits[location_index];
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
		case NEW_ACTION_EXPERIMENT_MEASURE:
			{
				for (int i_index = 0; i_index < history->instance_count; i_index++) {
					double final_score = (target_val - run_helper.result) / history->instance_count;
					this->test_location_scores[history->test_location_index] += final_score;
					this->test_location_counts[history->test_location_index]++;
				}
				this->test_location_truth_counts[history->test_location_index]++;

				if (this->test_location_counts[history->test_location_index] >= NEW_ACTION_NUM_DATAPOINTS
						&& this->test_location_truth_counts[history->test_location_index] >= NEW_ACTION_TRUTH_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					if (this->test_location_scores[history->test_location_index] > 0.0
							&& this->test_location_existing_impacts[history->test_location_index] >= 0.0) {
					#endif /* MDEBUG */
						this->test_location_scores[history->test_location_index] = 0.0;
						this->test_location_existing_impacts[history->test_location_index] = 0.0;
						this->test_location_counts[history->test_location_index] = 0;
						this->test_location_truth_counts[history->test_location_index] = 0;
						this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_1ST;
					} else {
						is_fail = true;
					}
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_1ST:
			{
				for (int i_index = 0; i_index < history->instance_count; i_index++) {
					double final_score = (target_val - run_helper.result) / history->instance_count;
					this->test_location_scores[history->test_location_index] += final_score;
					this->test_location_counts[history->test_location_index]++;
				}
				this->test_location_truth_counts[history->test_location_index]++;

				if (this->test_location_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS
						&& this->test_location_truth_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_TRUTH_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					if (this->test_location_scores[history->test_location_index] > 0.0
							&& this->test_location_existing_impacts[history->test_location_index] >= 0.0) {
					#endif /* MDEBUG */
						this->test_location_scores[history->test_location_index] = 0.0;
						this->test_location_existing_impacts[history->test_location_index] = 0.0;
						this->test_location_counts[history->test_location_index] = 0;
						this->test_location_truth_counts[history->test_location_index] = 0;
						this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_2ND;
					} else {
						is_fail = true;
					}
				}
			}

			break;
		case NEW_ACTION_EXPERIMENT_VERIFY_2ND:
			{
				for (int i_index = 0; i_index < history->instance_count; i_index++) {
					double final_score = (target_val - run_helper.result) / history->instance_count;
					this->test_location_scores[history->test_location_index] += final_score;
					this->test_location_counts[history->test_location_index]++;
				}
				this->test_location_truth_counts[history->test_location_index]++;

				if (this->test_location_counts[history->test_location_index] >= NEW_ACTION_VERIFY_2ND_NUM_DATAPOINTS
						&& this->test_location_truth_counts[history->test_location_index] >= NEW_ACTION_VERIFY_2ND_TRUTH_NUM_DATAPOINTS) {
					#if defined(MDEBUG) && MDEBUG
					if (rand()%2 == 0) {
					#else
					if (this->test_location_scores[history->test_location_index] > 0.0
							&& this->test_location_existing_impacts[history->test_location_index] >= 0.0) {
					#endif /* MDEBUG */
						ScopeNode* new_scope_node = this->test_scope_nodes[history->test_location_index];
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
						this->test_location_scores.erase(this->test_location_scores.begin() + history->test_location_index);
						this->test_location_existing_impacts.erase(this->test_location_existing_impacts.begin() + history->test_location_index);
						this->test_location_counts.erase(this->test_location_counts.begin() + history->test_location_index);
						this->test_location_truth_counts.erase(this->test_location_truth_counts.begin() + history->test_location_index);
						this->test_scope_nodes.erase(this->test_scope_nodes.begin() + history->test_location_index);

						this->generalize_iter++;
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
		this->test_location_scores.erase(this->test_location_scores.begin() + history->test_location_index);
		this->test_location_existing_impacts.erase(this->test_location_existing_impacts.begin() + history->test_location_index);
		this->test_location_counts.erase(this->test_location_counts.begin() + history->test_location_index);
		this->test_location_truth_counts.erase(this->test_location_truth_counts.begin() + history->test_location_index);
		delete this->test_scope_nodes[history->test_location_index];
		this->test_scope_nodes.erase(this->test_scope_nodes.begin() + history->test_location_index);

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
