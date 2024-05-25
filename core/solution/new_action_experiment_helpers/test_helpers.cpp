// #include "new_action_experiment.h"

// #include <iostream>

// #include "abstract_node.h"
// #include "constants.h"
// #include "eval.h"
// #include "globals.h"
// #include "scope.h"
// #include "solution.h"

// using namespace std;

// #if defined(MDEBUG) && MDEBUG
// const int NEW_ACTION_NUM_DATAPOINTS = 10;
// const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 10;
// #else
// const int NEW_ACTION_NUM_DATAPOINTS = 100;
// const int NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS = 1000;
// #endif /* MDEBUG */

// void NewActionExperiment::test_activate(
// 		int location_index,
// 		AbstractNode*& curr_node,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper,
// 		NewActionExperimentHistory* history) {
// 	history->test_location_index = location_index;

// 	switch (this->test_location_states[location_index]) {
// 	case NEW_ACTION_EXPERIMENT_MEASURE_NEW:
// 	case NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW:
// 		if (run_helper.num_actions_limit == -1) {
// 			run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;
// 		}

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
// 		EvalHistory* eval_history,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper) {
// 	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_scope_history->experiment_histories.back();

// 	if (run_helper.num_actions_limit == 0) {
// 		run_helper.num_actions_limit = -1;

// 		int experiment_index;
// 		for (int e_index = 0; e_index < (int)this->test_location_starts[history->test_location_index]->experiments.size(); e_index++) {
// 			if (this->test_location_starts[history->test_location_index]->experiments[e_index] == this) {
// 				experiment_index = e_index;
// 				break;
// 			}
// 		}
// 		this->test_location_starts[history->test_location_index]->experiments.erase(
// 			this->test_location_starts[history->test_location_index]->experiments.begin() + experiment_index);

// 		this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
// 		this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
// 		this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
// 		this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
// 		this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
// 		this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
// 		this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
// 		this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

// 		if (this->generalize_iter == -1
// 				&& this->successful_location_starts.size() == 0) {
// 			this->result = EXPERIMENT_RESULT_FAIL;
// 			/**
// 			 * - only continue if first succeeds
// 			 */
// 		} else {
// 			this->generalize_iter++;
// 			if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
// 				if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
// 					#if defined(MDEBUG) && MDEBUG
// 					for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
// 						int experiment_index;
// 						for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
// 							if (this->test_location_starts[t_index]->experiments[e_index] == this) {
// 								experiment_index = e_index;
// 								break;
// 							}
// 						}
// 						this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
// 					}
// 					this->test_location_starts.clear();
// 					this->test_location_is_branch.clear();
// 					this->test_location_exits.clear();
// 					this->test_location_states.clear();
// 					this->test_location_existing_scores.clear();
// 					this->test_location_existing_counts.clear();
// 					this->test_location_new_scores.clear();
// 					this->test_location_new_counts.clear();

// 					this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
// 					this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

// 					this->state = NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY;
// 					this->state_iter = 0;
// 					#else
// 					this->result = EXPERIMENT_RESULT_SUCCESS;
// 					#endif /* MDEBUG */
// 				} else {
// 					this->result = EXPERIMENT_RESULT_FAIL;
// 				}
// 			}
// 		}
// 	} else {
// 		this->scope_context->eval->activate(problem,
// 											run_helper,
// 											eval_history->end_scope_history);
// 		double predicted_impact = this->scope_context->eval->calc_vs(
// 			run_helper,
// 			eval_history);

// 		run_helper.num_actions_limit = -1;

// 		switch (this->test_location_states[history->test_location_index]) {
// 		case NEW_ACTION_EXPERIMENT_MEASURE_EXISTING:
// 			this->test_location_existing_scores[history->test_location_index] += predicted_impact;
// 			this->test_location_existing_counts[history->test_location_index]++;

// 			if (this->test_location_existing_counts[history->test_location_index] >= NEW_ACTION_NUM_DATAPOINTS) {
// 				this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_MEASURE_NEW;
// 			}
// 			break;
// 		case NEW_ACTION_EXPERIMENT_MEASURE_NEW:
// 			this->test_location_new_scores[history->test_location_index] += predicted_impact;
// 			this->test_location_new_counts[history->test_location_index]++;

// 			if (this->test_location_new_counts[history->test_location_index] >= NEW_ACTION_NUM_DATAPOINTS) {
// 				#if defined(MDEBUG) && MDEBUG
// 				if (rand()%2 == 0) {
// 				#else
// 				double existing_score = this->test_location_existing_scores[history->test_location_index]
// 					/ this->test_location_existing_counts[history->test_location_index];
// 				double new_score = this->test_location_new_scores[history->test_location_index]
// 					/ this->test_location_new_counts[history->test_location_index];

// 				if (new_score >= existing_score) {
// 				#endif /* MDEBUG */
// 					this->test_location_existing_scores[history->test_location_index] = 0.0;
// 					this->test_location_existing_counts[history->test_location_index] = 0;
// 					this->test_location_new_scores[history->test_location_index] = 0.0;
// 					this->test_location_new_counts[history->test_location_index] = 0;
// 					this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_1ST_EXISTING;
// 				} else {
// 					int experiment_index;
// 					for (int e_index = 0; e_index < (int)this->test_location_starts[history->test_location_index]->experiments.size(); e_index++) {
// 						if (this->test_location_starts[history->test_location_index]->experiments[e_index] == this) {
// 							experiment_index = e_index;
// 							break;
// 						}
// 					}
// 					this->test_location_starts[history->test_location_index]->experiments.erase(
// 						this->test_location_starts[history->test_location_index]->experiments.begin() + experiment_index);
// 					/**
// 					 * - can simply remove first
// 					 */

// 					this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
// 					this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
// 					this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
// 					this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
// 					this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
// 					this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
// 					this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
// 					this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

// 					if (this->generalize_iter == -1
// 							&& this->successful_location_starts.size() == 0) {
// 						this->result = EXPERIMENT_RESULT_FAIL;
// 						/**
// 						 * - only continue if first succeeds
// 						 */
// 					} else {
// 						this->generalize_iter++;
// 						if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
// 							if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
// 								#if defined(MDEBUG) && MDEBUG
// 								for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
// 									int experiment_index;
// 									for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
// 										if (this->test_location_starts[t_index]->experiments[e_index] == this) {
// 											experiment_index = e_index;
// 											break;
// 										}
// 									}
// 									this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
// 								}
// 								this->test_location_starts.clear();
// 								this->test_location_is_branch.clear();
// 								this->test_location_exits.clear();
// 								this->test_location_states.clear();
// 								this->test_location_existing_scores.clear();
// 								this->test_location_existing_counts.clear();
// 								this->test_location_new_scores.clear();
// 								this->test_location_new_counts.clear();

// 								this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
// 								this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

// 								this->state = NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY;
// 								this->state_iter = 0;
// 								#else
// 								this->result = EXPERIMENT_RESULT_SUCCESS;
// 								#endif /* MDEBUG */
// 							} else {
// 								this->result = EXPERIMENT_RESULT_FAIL;
// 							}
// 						}
// 					}
// 				}
// 			}

// 			break;
// 		case NEW_ACTION_EXPERIMENT_VERIFY_1ST_EXISTING:
// 			this->test_location_existing_scores[history->test_location_index] += predicted_impact;
// 			this->test_location_existing_counts[history->test_location_index]++;

// 			if (this->test_location_existing_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS) {
// 				this->test_location_states[history->test_location_index] = NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW;
// 			}
// 			break;
// 		case NEW_ACTION_EXPERIMENT_VERIFY_1ST_NEW:
// 			this->test_location_new_scores[history->test_location_index] += predicted_impact;
// 			this->test_location_new_counts[history->test_location_index]++;

// 			if (this->test_location_new_counts[history->test_location_index] >= NEW_ACTION_VERIFY_1ST_NUM_DATAPOINTS) {
// 				#if defined(MDEBUG) && MDEBUG
// 				if (rand()%2 == 0) {
// 				#else
// 				double existing_score = this->test_location_existing_scores[history->test_location_index]
// 					/ this->test_location_existing_counts[history->test_location_index];
// 				double new_score = this->test_location_new_scores[history->test_location_index]
// 					/ this->test_location_new_counts[history->test_location_index];

// 				if (new_score >= existing_score) {
// 				#endif /* MDEBUG */
// 					this->successful_location_starts.push_back(this->test_location_starts[history->test_location_index]);
// 					this->successful_location_is_branch.push_back(this->test_location_is_branch[history->test_location_index]);
// 					this->successful_location_exits.push_back(this->test_location_exits[history->test_location_index]);
// 				} else {
// 					int experiment_index;
// 					for (int e_index = 0; e_index < (int)this->test_location_starts[history->test_location_index]->experiments.size(); e_index++) {
// 						if (this->test_location_starts[history->test_location_index]->experiments[e_index] == this) {
// 							experiment_index = e_index;
// 							break;
// 						}
// 					}
// 					this->test_location_starts[history->test_location_index]->experiments.erase(
// 						this->test_location_starts[history->test_location_index]->experiments.begin() + experiment_index);
// 					/**
// 					 * - can simply remove first
// 					 */
// 				}

// 				this->test_location_starts.erase(this->test_location_starts.begin() + history->test_location_index);
// 				this->test_location_is_branch.erase(this->test_location_is_branch.begin() + history->test_location_index);
// 				this->test_location_exits.erase(this->test_location_exits.begin() + history->test_location_index);
// 				this->test_location_states.erase(this->test_location_states.begin() + history->test_location_index);
// 				this->test_location_existing_scores.erase(this->test_location_existing_scores.begin() + history->test_location_index);
// 				this->test_location_existing_counts.erase(this->test_location_existing_counts.begin() + history->test_location_index);
// 				this->test_location_new_scores.erase(this->test_location_new_scores.begin() + history->test_location_index);
// 				this->test_location_new_counts.erase(this->test_location_new_counts.begin() + history->test_location_index);

// 				if (this->generalize_iter == -1
// 						&& this->successful_location_starts.size() == 0) {
// 					this->result = EXPERIMENT_RESULT_FAIL;
// 					/**
// 					 * - only continue if first succeeds
// 					 */
// 				} else {
// 					this->generalize_iter++;
// 					if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
// 						if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
// 							#if defined(MDEBUG) && MDEBUG
// 							for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
// 								int experiment_index;
// 								for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
// 									if (this->test_location_starts[t_index]->experiments[e_index] == this) {
// 										experiment_index = e_index;
// 										break;
// 									}
// 								}
// 								this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
// 							}
// 							this->test_location_starts.clear();
// 							this->test_location_is_branch.clear();
// 							this->test_location_exits.clear();
// 							this->test_location_states.clear();
// 							this->test_location_existing_scores.clear();
// 							this->test_location_existing_counts.clear();
// 							this->test_location_new_scores.clear();
// 							this->test_location_new_counts.clear();

// 							this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
// 							this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

// 							this->state = NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY;
// 							this->state_iter = 0;
// 							#else
// 							this->result = EXPERIMENT_RESULT_SUCCESS;
// 							#endif /* MDEBUG */
// 						} else {
// 							this->result = EXPERIMENT_RESULT_FAIL;
// 						}
// 					}
// 				}
// 			}

// 			break;
// 		}
// 	}
// }
