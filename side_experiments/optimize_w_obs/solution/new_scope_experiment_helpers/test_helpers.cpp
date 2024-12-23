#include "new_scope_experiment.h"

#include <iostream>

#include "globals.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
const int NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 2;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 50;
const int NEW_SCOPE_TRUTH_NUM_DATAPOINTS = 20;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 250;
const int NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS = 100;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 1000;
const int NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS = 400;
#endif /* MDEBUG */

void NewScopeExperiment::test_activate(
		int location_index,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	if (history->test_location_index == -1) {
		history->test_location_index = location_index;
		history->instance_count = 0;
	}

	history->instance_count++;

	context.back().node_id = -1;

	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	this->new_scope->new_scope_activate(problem,
										context,
										run_helper,
										inner_scope_history);
	delete inner_scope_history;

	curr_node = this->test_location_exits[location_index];
}

void NewScopeExperiment::test_backprop(
		double target_val,
		RunHelper& run_helper) {
	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)run_helper.experiment_histories.back();

	bool is_fail = false;

	switch (this->test_location_states[history->test_location_index]) {
	case NEW_SCOPE_EXPERIMENT_MEASURE:
		{
			for (int i_index = 0; i_index < history->instance_count; i_index++) {
				double final_score = (target_val - run_helper.result) / history->instance_count;
				this->test_location_scores[history->test_location_index] += final_score;
				this->test_location_counts[history->test_location_index]++;
			}
			this->test_location_truth_counts[history->test_location_index]++;

			if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_NUM_DATAPOINTS
					&& this->test_location_truth_counts[history->test_location_index] >= NEW_SCOPE_TRUTH_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->test_location_scores[history->test_location_index] > 0.0) {
				#endif /* MDEBUG */
					this->test_location_scores[history->test_location_index] = 0.0;
					this->test_location_counts[history->test_location_index] = 0;
					this->test_location_truth_counts[history->test_location_index] = 0;
					this->test_location_states[history->test_location_index] = NEW_SCOPE_EXPERIMENT_VERIFY_1ST;
				} else {
					is_fail = true;
				}
			}
		}

		break;
	case NEW_SCOPE_EXPERIMENT_VERIFY_1ST:
		{
			for (int i_index = 0; i_index < history->instance_count; i_index++) {
				double final_score = (target_val - run_helper.result) / history->instance_count;
				this->test_location_scores[history->test_location_index] += final_score;
				this->test_location_counts[history->test_location_index]++;
			}
			this->test_location_truth_counts[history->test_location_index]++;

			if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS
					&& this->test_location_truth_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_1ST_TRUTH_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->test_location_scores[history->test_location_index] > 0.0) {
				#endif /* MDEBUG */
					this->test_location_scores[history->test_location_index] = 0.0;
					this->test_location_counts[history->test_location_index] = 0;
					this->test_location_truth_counts[history->test_location_index] = 0;
					this->test_location_states[history->test_location_index] = NEW_SCOPE_EXPERIMENT_VERIFY_2ND;
				} else {
					is_fail = true;
				}
			}
		}

		break;
	case NEW_SCOPE_EXPERIMENT_VERIFY_2ND:
		{
			for (int i_index = 0; i_index < history->instance_count; i_index++) {
				double final_score = (target_val - run_helper.result) / history->instance_count;
				this->test_location_scores[history->test_location_index] += final_score;
				this->test_location_counts[history->test_location_index]++;
			}
			this->test_location_truth_counts[history->test_location_index]++;

			if (this->test_location_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS
					&& this->test_location_truth_counts[history->test_location_index] >= NEW_SCOPE_VERIFY_2ND_TRUTH_NUM_DATAPOINTS) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->test_location_scores[history->test_location_index] > 0.0) {
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
