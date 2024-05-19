// #include "new_action_experiment.h"

// #include <iostream>

// #include "abstract_node.h"
// #include "constants.h"
// #include "globals.h"
// #include "scope.h"
// #include "solution.h"

// using namespace std;

// void NewActionExperiment::test_activate(
// 		int location_index,
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		NewActionExperimentHistory* history) {
// 	history->test_location_index = location_index;

// 	if (this->test_location_states[location_index] == NEW_ACTION_EXPERIMENT_MEASURE_NEW) {
// 		context.push_back(ContextLayer());

// 		context.back().scope = this->scope_context;
// 		context.back().node = NULL;

// 		ScopeHistory* scope_history = new ScopeHistory(this->scope_context);
// 		context.back().scope_history = scope_history;

// 		this->scope_context->new_action_activate(this->starting_node,
// 												 this->included_nodes,
// 												 problem,
// 												 context,
// 												 run_helper,
// 												 scope_history);

// 		delete scope_history;

// 		context.pop_back();

// 		curr_node = this->test_location_exits[location_index];
// 	}
// }

// void NewActionExperiment::test_backprop(
// 		double target_val,
// 		RunHelper& run_helper) {
// 	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories.back();

// 	if (!run_helper.exceeded_limit) {
// 		if (run_helper.max_depth > solution->max_depth) {
// 			solution->max_depth = run_helper.max_depth;
// 		}

// 		if (run_helper.num_actions > solution->max_num_actions) {
// 			solution->max_num_actions = run_helper.num_actions;
// 		}
// 	}

// 	if (this->test_location_states[history->test_location_index] == NEW_ACTION_EXPERIMENT_MEASURE_EXISTING) {
// 		this->test_location_existing_scores[history->test_location_index] += target_val;
// 		this->test_location_existing_counts[history->test_location_index]++;

// 		if (this->test_location_existing_counts[history->test_location_index] >= NUM_DATAPOINTS) {
// 			this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_MEASURE_NEW;
// 		}
// 	} else {
// 		this->test_location_new_scores[history->test_location_index] += target_val;
// 		this->test_location_new_counts[history->test_location_index]++;

// 		if (this->test_location_new_counts[history->test_location_index] >= NUM_DATAPOINTS) {
// 			#if defined(MDEBUG) && MDEBUG
// 			if (rand()%2 == 0) {
// 			#else
// 			double existing_score = this->test_location_existing_scores[history->test_location_index]
// 				/ this->test_location_existing_counts[history->test_location_index];
// 			double new_score = this->test_location_new_scores[history->test_location_index]
// 				/ this->test_location_new_counts[history->test_location_index];

// 			if (new_score >= existing_score) {
// 			#endif /* MDEBUG */
// 				this->successful_location_starts.push_back(this->test_location_starts[history->test_location_index]);
// 				this->successful_location_is_branch.push_back(this->test_location_is_branch[history->test_location_index]);
// 				this->successful_location_exits.push_back(this->test_location_exits[history->test_location_index]);
// 			} else {
// 				int experiment_index;
// 				for (int e_index = 0; e_index < (int)this->test_location_starts[history->test_location_index]->experiments.size(); e_index++) {
// 					if (this->test_location_starts[history->test_location_index]->experiments[e_index] == this) {
// 						experiment_index = e_index;
// 						break;
// 					}
// 				}
// 				this->test_location_starts[history->test_location_index]->experiments.erase(
// 					this->test_location_starts[history->test_location_index]->experiments.begin() + experiment_index);
// 				/**
// 				 * - can simply remove first
// 				 */
// 			}

// 			this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
// 			this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
// 			this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
// 			this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
// 			this->test_location_state_iters.erase(this->test_location_state_iters.begin() + history->test_location_index);
// 			this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
// 			this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
// 			this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
// 			this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

// 			if (this->generalize_iter == -1
// 					&& this->successful_location_starts.size() == 0) {
// 				this->result = EXPERIMENT_RESULT_FAIL;
// 				/**
// 				 * - only continue if first succeeds
// 				 */
// 			} else {
// 				this->generalize_iter++;
// 				if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
// 					if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
// 						this->result = EXPERIMENT_RESULT_SUCCESS;
// 					} else {
// 						this->result = EXPERIMENT_RESULT_FAIL;
// 					}
// 				}
// 			}
// 		}
// 	}
// }
