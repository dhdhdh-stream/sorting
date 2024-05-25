// #include "branch_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "branch_node.h"
// #include "constants.h"
// #include "eval.h"
// #include "globals.h"
// #include "info_branch_node.h"
// #include "network.h"
// #include "new_info_experiment.h"
// #include "pass_through_experiment.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"
// #include "solution_helpers.h"
// #include "utilities.h"

// using namespace std;

// bool BranchExperiment::experiment_verify_activate(
// 		AbstractNode*& curr_node,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper) {
// 	if (this->is_pass_through) {
// 		if (this->best_step_types.size() == 0) {
// 			curr_node = this->best_exit_next_node;
// 		} else {
// 			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
// 				curr_node = this->best_actions[0];
// 			} else {
// 				curr_node = this->best_scopes[0];
// 			}
// 		}

// 		return true;
// 	} else {
// 		run_helper.num_decisions++;

// 		vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
// 		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 			int curr_layer = 0;
// 			ScopeHistory* curr_scope_history = context.back().scope_history;
// 			while (true) {
// 				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
// 					this->input_node_contexts[i_index][curr_layer]);
// 				if (it == curr_scope_history->node_histories.end()) {
// 					break;
// 				} else {
// 					if (curr_layer == (int)this->input_scope_contexts[i_index].size()-1) {
// 						switch (it->first->type) {
// 						case NODE_TYPE_ACTION:
// 							{
// 								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
// 								input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
// 							}
// 							break;
// 						case NODE_TYPE_BRANCH:
// 							{
// 								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
// 								if (branch_node_history->is_branch) {
// 									input_vals[i_index] = 1.0;
// 								} else {
// 									input_vals[i_index] = -1.0;
// 								}
// 							}
// 							break;
// 						case NODE_TYPE_INFO_BRANCH:
// 							{
// 								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
// 								if (info_branch_node_history->is_branch) {
// 									input_vals[i_index] = 1.0;
// 								} else {
// 									input_vals[i_index] = -1.0;
// 								}
// 							}
// 							break;
// 						}
// 						break;
// 					} else {
// 						curr_layer++;
// 						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
// 					}
// 				}
// 			}
// 		}

// 		double existing_predicted_score = this->existing_average_score;
// 		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 			existing_predicted_score += input_vals[i_index] * this->existing_linear_weights[i_index];
// 		}
// 		if (this->existing_network != NULL) {
// 			vector<vector<double>> existing_network_input_vals(this->existing_network_input_indexes.size());
// 			for (int i_index = 0; i_index < (int)this->existing_network_input_indexes.size(); i_index++) {
// 				existing_network_input_vals[i_index] = vector<double>(this->existing_network_input_indexes[i_index].size());
// 				for (int s_index = 0; s_index < (int)this->existing_network_input_indexes[i_index].size(); s_index++) {
// 					existing_network_input_vals[i_index][s_index] = input_vals[this->existing_network_input_indexes[i_index][s_index]];
// 				}
// 			}
// 			this->existing_network->activate(existing_network_input_vals);
// 			existing_predicted_score += this->existing_network->output->acti_vals[0];
// 		}

// 		double new_predicted_score = this->new_average_score;
// 		for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
// 			new_predicted_score += input_vals[i_index] * this->new_linear_weights[i_index];
// 		}
// 		if (this->new_network != NULL) {
// 			vector<vector<double>> new_network_input_vals(this->new_network_input_indexes.size());
// 			for (int i_index = 0; i_index < (int)this->new_network_input_indexes.size(); i_index++) {
// 				new_network_input_vals[i_index] = vector<double>(this->new_network_input_indexes[i_index].size());
// 				for (int s_index = 0; s_index < (int)this->new_network_input_indexes[i_index].size(); s_index++) {
// 					new_network_input_vals[i_index][s_index] = input_vals[this->new_network_input_indexes[i_index][s_index]];
// 				}
// 			}
// 			this->new_network->activate(new_network_input_vals);
// 			new_predicted_score += this->new_network->output->acti_vals[0];
// 		}

// 		#if defined(MDEBUG) && MDEBUG
// 		bool decision_is_branch;
// 		if (run_helper.curr_run_seed%2 == 0) {
// 			decision_is_branch = true;
// 		} else {
// 			decision_is_branch = false;
// 		}
// 		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
// 		#else
// 		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
// 		#endif /* MDEBUG */

// 		if (decision_is_branch) {
// 			if (this->best_step_types.size() == 0) {
// 				curr_node = this->best_exit_next_node;
// 			} else {
// 				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
// 					curr_node = this->best_actions[0];
// 				} else {
// 					curr_node = this->best_scopes[0];
// 				}
// 			}

// 			return true;
// 		} else {
// 			return false;
// 		}
// 	}
// }

// void BranchExperiment::experiment_verify_backprop(
// 		EvalHistory* eval_history,
// 		Problem* problem,
// 		vector<ContextLayer>& context,
// 		RunHelper& run_helper) {
// 	this->scope_context->eval->activate(problem,
// 										run_helper,
// 										eval_history->end_scope_history);
// 	double predicted_impact = this->scope_context->eval->calc_vs(
// 		run_helper,
// 		eval_history);
// 	this->combined_score += predicted_impact;

// 	this->state_iter++;
// 	if (this->root_state == ROOT_EXPERIMENT_STATE_VERIFY_1ST
// 			&& this->state_iter >= VERIFY_1ST_NUM_DATAPOINTS) {
// 		this->combined_score /= VERIFY_1ST_NUM_DATAPOINTS;

// 		#if defined(MDEBUG) && MDEBUG
// 		if (rand()%2 == 0) {
// 		#else
// 		if (this->combined_score > this->verify_existing_average_score) {
// 		#endif /* MDEBUG */
// 			this->target_val_histories.reserve(VERIFY_2ND_NUM_DATAPOINTS);

// 			this->root_state = ROOT_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
// 			/**
// 			 * - leave this->experiment_iter unchanged
// 			 */
// 		} else {
// 			bool to_experiment = false;
// 			switch (this->verify_experiments.back()->type) {
// 			case EXPERIMENT_TYPE_BRANCH:
// 				{
// 					double combined_branch_weight = 1.0;
// 					AbstractExperiment* curr_experiment = this->verify_experiments.back();
// 					while (true) {
// 						if (curr_experiment == NULL) {
// 							break;
// 						}

// 						switch (curr_experiment->type) {
// 						case EXPERIMENT_TYPE_BRANCH:
// 							{
// 								BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
// 								combined_branch_weight *= branch_experiment->branch_weight;
// 							}
// 							break;
// 						// case EXPERIMENT_TYPE_NEW_INFO:
// 						// 	{
// 						// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
// 						// 		combined_branch_weight *= new_info_experiment->branch_weight;
// 						// 	}
// 						// 	break;
// 						}
// 						curr_experiment = curr_experiment->parent_experiment;
// 					}

// 					BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
// 					if (branch_experiment->best_step_types.size() > 0
// 							&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
// 						#if defined(MDEBUG) && MDEBUG
// 						for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
// 							delete branch_experiment->verify_problems[p_index];
// 						}
// 						branch_experiment->verify_problems.clear();
// 						branch_experiment->verify_seeds.clear();
// 						branch_experiment->verify_original_scores.clear();
// 						branch_experiment->verify_branch_scores.clear();
// 						/**
// 						 * - simply rely on leaf experiment to verify
// 						 */
// 						#endif /* MDEBUG */

// 						branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
// 						branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 						branch_experiment->experiment_iter = 0;

// 						to_experiment = true;
// 					}
// 				}
// 				break;
// 			// case EXPERIMENT_TYPE_PASS_THROUGH:
// 			// 	{
// 			// 		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
// 			// 		if (pass_through_experiment->best_step_types.size() > 0) {
// 			// 			pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			pass_through_experiment->experiment_iter = 0;

// 			// 			to_experiment = true;
// 			// 		}
// 			// 	}
// 			// 	break;
// 			// case EXPERIMENT_TYPE_NEW_INFO:
// 			// 	{
// 			// 		double combined_branch_weight = 1.0;
// 			// 		AbstractExperiment* curr_experiment = this->verify_experiments.back();
// 			// 		while (true) {
// 			// 			if (curr_experiment == NULL) {
// 			// 				break;
// 			// 			}

// 			// 			switch (curr_experiment->type) {
// 			// 			case EXPERIMENT_TYPE_BRANCH:
// 			// 				{
// 			// 					BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
// 			// 					combined_branch_weight *= branch_experiment->branch_weight;
// 			// 				}
// 			// 				break;
// 			// 			case EXPERIMENT_TYPE_NEW_INFO:
// 			// 				{
// 			// 					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
// 			// 					combined_branch_weight *= new_info_experiment->branch_weight;
// 			// 				}
// 			// 				break;
// 			// 			}
// 			// 			curr_experiment = curr_experiment->parent_experiment;
// 			// 		}

// 			// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)this->verify_experiments.back();
// 			// 		if (new_info_experiment->best_step_types.size() > 0
// 			// 				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
// 			// 			#if defined(MDEBUG) && MDEBUG
// 			// 			for (int p_index = 0; p_index < (int)new_info_experiment->verify_problems.size(); p_index++) {
// 			// 				delete new_info_experiment->verify_problems[p_index];
// 			// 			}
// 			// 			new_info_experiment->verify_problems.clear();
// 			// 			new_info_experiment->verify_seeds.clear();
// 			// 			new_info_experiment->verify_negative_scores.clear();
// 			// 			new_info_experiment->verify_positive_scores.clear();
// 			// 			#endif /* MDEBUG */

// 			// 			new_info_experiment->state = NEW_INFO_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			new_info_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			new_info_experiment->experiment_iter = 0;

// 			// 			to_experiment = true;
// 			// 		}
// 			// 	}
// 			// 	break;
// 			}

// 			if (!to_experiment) {
// 				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

// 				curr_experiment->experiment_iter++;
// 				int matching_index;
// 				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
// 					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
// 						matching_index = c_index;
// 						break;
// 					}
// 				}
// 				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

// 				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
// 				this->verify_experiments.back()->finalize(NULL);
// 				delete this->verify_experiments.back();

// 				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
// 					* pow(0.5, this->verify_experiments.size());
// 				while (true) {
// 					if (curr_experiment->parent_experiment == NULL) {
// 						break;
// 					}

// 					if (curr_experiment->experiment_iter >= target_count) {
// 						AbstractExperiment* parent = curr_experiment->parent_experiment;

// 						parent->experiment_iter++;
// 						int matching_index;
// 						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
// 							if (parent->child_experiments[c_index] == curr_experiment) {
// 								matching_index = c_index;
// 								break;
// 							}
// 						}
// 						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

// 						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
// 						curr_experiment->finalize(NULL);
// 						delete curr_experiment;

// 						curr_experiment = parent;
// 						target_count *= 2.0;
// 					} else {
// 						break;
// 					}
// 				}
// 			}

// 			this->verify_experiments.clear();

// 			if (this->experiment_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
// 				this->result = EXPERIMENT_RESULT_FAIL;
// 			} else {
// 				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			}
// 		}
// 	} else if (this->state_iter >= VERIFY_2ND_NUM_DATAPOINTS) {
// 		this->combined_score /= VERIFY_2ND_NUM_DATAPOINTS;

// 		#if defined(MDEBUG) && MDEBUG
// 		if (rand()%2 == 0) {
// 		#else
// 		if (this->combined_score > this->verify_existing_average_score) {
// 		#endif /* MDEBUG */
// 			cout << "experiment success" << endl;
// 			cout << "this->scope_context->id: " << this->scope_context->id << endl;
// 			cout << "this->node_context->id: " << this->node_context->id << endl;
// 			cout << "this->is_branch: " << this->is_branch << endl;
// 			cout << "new explore path:";
// 			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
// 				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
// 					cout << " " << this->best_actions[s_index]->action.move;
// 				} else {
// 					cout << " E";
// 				}
// 			}
// 			cout << endl;

// 			if (this->best_exit_next_node == NULL) {
// 				cout << "this->best_exit_next_node->id: " << -1 << endl;
// 			} else {
// 				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
// 			}

// 			cout << "this->combined_score: " << this->combined_score << endl;
// 			cout << "this->verify_existing_average_score: " << this->verify_existing_average_score << endl;

// 			/**
// 			 * - also finalize this->verify_experiments in finalize()
// 			 */

// 			this->result = EXPERIMENT_RESULT_SUCCESS;
// 		} else {
// 			bool to_experiment = false;
// 			switch (this->verify_experiments.back()->type) {
// 			case EXPERIMENT_TYPE_BRANCH:
// 				{
// 					double combined_branch_weight = 1.0;
// 					AbstractExperiment* curr_experiment = this->verify_experiments.back();
// 					while (true) {
// 						if (curr_experiment == NULL) {
// 							break;
// 						}

// 						switch (curr_experiment->type) {
// 						case EXPERIMENT_TYPE_BRANCH:
// 							{
// 								BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
// 								combined_branch_weight *= branch_experiment->branch_weight;
// 							}
// 							break;
// 						// case EXPERIMENT_TYPE_NEW_INFO:
// 						// 	{
// 						// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
// 						// 		combined_branch_weight *= new_info_experiment->branch_weight;
// 						// 	}
// 						// 	break;
// 						}
// 						curr_experiment = curr_experiment->parent_experiment;
// 					}

// 					BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
// 					if (branch_experiment->best_step_types.size() > 0
// 							&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
// 						#if defined(MDEBUG) && MDEBUG
// 						for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
// 							delete branch_experiment->verify_problems[p_index];
// 						}
// 						branch_experiment->verify_problems.clear();
// 						branch_experiment->verify_seeds.clear();
// 						branch_experiment->verify_original_scores.clear();
// 						branch_experiment->verify_branch_scores.clear();
// 						/**
// 						 * - simply rely on leaf experiment to verify
// 						 */
// 						#endif /* MDEBUG */

// 						branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
// 						branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 						branch_experiment->experiment_iter = 0;

// 						to_experiment = true;
// 					}
// 				}
// 				break;
// 			// case EXPERIMENT_TYPE_PASS_THROUGH:
// 			// 	{
// 			// 		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
// 			// 		if (pass_through_experiment->best_step_types.size() > 0) {
// 			// 			pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			pass_through_experiment->experiment_iter = 0;

// 			// 			to_experiment = true;
// 			// 		}
// 			// 	}
// 			// 	break;
// 			// case EXPERIMENT_TYPE_NEW_INFO:
// 			// 	{
// 			// 		double combined_branch_weight = 1.0;
// 			// 		AbstractExperiment* curr_experiment = this->verify_experiments.back();
// 			// 		while (true) {
// 			// 			if (curr_experiment == NULL) {
// 			// 				break;
// 			// 			}

// 			// 			switch (curr_experiment->type) {
// 			// 			case EXPERIMENT_TYPE_BRANCH:
// 			// 				{
// 			// 					BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
// 			// 					combined_branch_weight *= branch_experiment->branch_weight;
// 			// 				}
// 			// 				break;
// 			// 			case EXPERIMENT_TYPE_NEW_INFO:
// 			// 				{
// 			// 					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
// 			// 					combined_branch_weight *= new_info_experiment->branch_weight;
// 			// 				}
// 			// 				break;
// 			// 			}
// 			// 			curr_experiment = curr_experiment->parent_experiment;
// 			// 		}

// 			// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)this->verify_experiments.back();
// 			// 		if (new_info_experiment->best_step_types.size() > 0
// 			// 				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
// 			// 			#if defined(MDEBUG) && MDEBUG
// 			// 			for (int p_index = 0; p_index < (int)new_info_experiment->verify_problems.size(); p_index++) {
// 			// 				delete new_info_experiment->verify_problems[p_index];
// 			// 			}
// 			// 			new_info_experiment->verify_problems.clear();
// 			// 			new_info_experiment->verify_seeds.clear();
// 			// 			new_info_experiment->verify_negative_scores.clear();
// 			// 			new_info_experiment->verify_positive_scores.clear();
// 			// 			#endif /* MDEBUG */

// 			// 			new_info_experiment->state = NEW_INFO_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			new_info_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			// 			new_info_experiment->experiment_iter = 0;

// 			// 			to_experiment = true;
// 			// 		}
// 			// 	}
// 			// 	break;
// 			}

// 			if (!to_experiment) {
// 				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

// 				curr_experiment->experiment_iter++;
// 				int matching_index;
// 				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
// 					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
// 						matching_index = c_index;
// 						break;
// 					}
// 				}
// 				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

// 				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
// 				this->verify_experiments.back()->finalize(NULL);
// 				delete this->verify_experiments.back();

// 				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
// 					* pow(0.5, this->verify_experiments.size());
// 				while (true) {
// 					if (curr_experiment->parent_experiment == NULL) {
// 						break;
// 					}

// 					if (curr_experiment->experiment_iter >= target_count) {
// 						AbstractExperiment* parent = curr_experiment->parent_experiment;

// 						parent->experiment_iter++;
// 						int matching_index;
// 						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
// 							if (parent->child_experiments[c_index] == curr_experiment) {
// 								matching_index = c_index;
// 								break;
// 							}
// 						}
// 						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

// 						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
// 						curr_experiment->finalize(NULL);
// 						delete curr_experiment;

// 						curr_experiment = parent;
// 						target_count *= 2.0;
// 					} else {
// 						break;
// 					}
// 				}
// 			}

// 			this->verify_experiments.clear();

// 			if (this->experiment_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
// 				this->result = EXPERIMENT_RESULT_FAIL;
// 			} else {
// 				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
// 			}
// 		}
// 	}
// }
