#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "new_info_experiment.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::experiment_verify_activate(
		AbstractNode*& curr_node,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		BranchExperimentHistory* history) {
	history->starting_predicted_scores.push_back(vector<double>(context.size(), 0.0));
	history->normalized_scores.push_back(vector<double>(context.size(), 0.0));
	for (int l_index = 0; l_index < (int)context.size(); l_index++) {
		if (context[l_index].scope->eval_network != NULL) {
			double starting_predicted_score = calc_score(context[l_index].scope_history);
			history->starting_predicted_scores.back()[l_index] = starting_predicted_score;

			context[l_index].scope_history->callback_experiment_history = history;
			context[l_index].scope_history->callback_experiment_indexes.push_back(
				(int)history->starting_predicted_scores.size()-1);
			context[l_index].scope_history->callback_experiment_layers.push_back(l_index);
		}
	}

	if (this->is_pass_through) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		run_helper.num_decisions++;

		vector<double> existing_input_vals(this->existing_input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->existing_input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->existing_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->existing_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								existing_input_vals[i_index] = action_node_history->obs_snapshot[this->existing_input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								if (branch_node_history->is_branch) {
									existing_input_vals[i_index] = 1.0;
								} else {
									existing_input_vals[i_index] = -1.0;
								}
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									existing_input_vals[i_index] = 1.0;
								} else {
									existing_input_vals[i_index] = -1.0;
								}
							}
							break;
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}
		this->existing_network->activate(existing_input_vals);
		#if defined(MDEBUG) && MDEBUG
		#else
		double existing_predicted_score = this->existing_network->output->acti_vals[0];
		#endif /* MDEBUG */

		vector<double> new_input_vals(this->new_input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			ScopeHistory* curr_scope_history = context.back().scope_history;
			while (true) {
				map<AbstractNode*, AbstractNodeHistory*>::iterator it = curr_scope_history->node_histories.find(
					this->new_input_node_contexts[i_index][curr_layer]);
				if (it == curr_scope_history->node_histories.end()) {
					break;
				} else {
					if (curr_layer == (int)this->new_input_scope_contexts[i_index].size()-1) {
						switch (it->first->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
								new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
								if (branch_node_history->is_branch) {
									new_input_vals[i_index] = 1.0;
								} else {
									new_input_vals[i_index] = -1.0;
								}
							}
							break;
						case NODE_TYPE_INFO_BRANCH:
							{
								InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
								if (info_branch_node_history->is_branch) {
									new_input_vals[i_index] = 1.0;
								} else {
									new_input_vals[i_index] = -1.0;
								}
							}
							break;
						}
						break;
					} else {
						curr_layer++;
						curr_scope_history = ((ScopeNodeHistory*)it->second)->scope_history;
					}
				}
			}
		}
		this->new_network->activate(new_input_vals);
		#if defined(MDEBUG) && MDEBUG
		#else
		double new_predicted_score = this->new_network->output->acti_vals[0];
		#endif /* MDEBUG */

		#if defined(MDEBUG) && MDEBUG
		bool decision_is_branch;
		if (run_helper.curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		bool decision_is_branch = new_predicted_score >= existing_predicted_score;
		#endif /* MDEBUG */

		BranchNodeHistory* branch_node_history = new BranchNodeHistory();
		branch_node_history->index = context.back().scope_history->node_histories.size();
		context.back().scope_history->node_histories[this->branch_node] = branch_node_history;
		if (decision_is_branch) {
			branch_node_history->is_branch = true;

			if (this->best_step_types.size() == 0) {
				curr_node = this->best_exit_next_node;
			} else {
				if (this->best_step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->best_actions[0];
				} else {
					curr_node = this->best_scopes[0];
				}
			}

			return true;
		} else {
			branch_node_history->is_branch = false;

			return false;
		}
	}
}

void BranchExperiment::experiment_verify_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	double ending_predicted_score;
	if (run_helper.num_actions > solution->num_actions_limit) {
		ending_predicted_score = -1.0;
	} else {
		ending_predicted_score = calc_score(context.back().scope_history);
	}
	for (int i_index = 0; i_index < (int)context.back().scope_history->callback_experiment_indexes.size(); i_index++) {
		double predicted_score = ending_predicted_score
			- history->starting_predicted_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
				[context.back().scope_history->callback_experiment_layers[i_index]];
		history->normalized_scores[context.back().scope_history->callback_experiment_indexes[i_index]]
			[context.back().scope_history->callback_experiment_layers[i_index]] = predicted_score / context.back().scope->eval_score_standard_deviation;
	}
}

void BranchExperiment::experiment_verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	double final_normalized_score = (target_val - solution->average_score) / solution->score_standard_deviation;
	for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->starting_predicted_scores[i_index].size(); l_index++) {
			sum_score += history->normalized_scores[i_index][l_index];
		}
		double final_score = sum_score / (int)history->starting_predicted_scores.size() + final_normalized_score;
		this->combined_score += final_score;
		this->state_iter++;
	}

	if (this->state_iter >= VERIFY_NUM_DATAPOINTS) {
		this->combined_score /= this->state_iter;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->combined_score > this->existing_average_score) {
		#endif /* MDEBUG */
			cout << "experiment success" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else {
					cout << " E";
				}
			}
			cout << endl;

			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->existing_average_score: " << this->existing_average_score << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			bool to_experiment = false;
			switch (this->verify_experiments.back()->type) {
			case EXPERIMENT_TYPE_BRANCH:
				{
					double combined_branch_weight = 1.0;
					AbstractExperiment* curr_experiment = this->verify_experiments.back();
					while (true) {
						if (curr_experiment == NULL) {
							break;
						}

						switch (curr_experiment->type) {
						case EXPERIMENT_TYPE_BRANCH:
							{
								BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
								combined_branch_weight *= branch_experiment->branch_weight;
							}
							break;
						// case EXPERIMENT_TYPE_NEW_INFO:
						// 	{
						// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
						// 		combined_branch_weight *= new_info_experiment->branch_weight;
						// 	}
						// 	break;
						}
						curr_experiment = curr_experiment->parent_experiment;
					}

					BranchExperiment* branch_experiment = (BranchExperiment*)this->verify_experiments.back();
					if (branch_experiment->best_step_types.size() > 0
							&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
						#if defined(MDEBUG) && MDEBUG
						for (int p_index = 0; p_index < (int)branch_experiment->verify_problems.size(); p_index++) {
							delete branch_experiment->verify_problems[p_index];
						}
						branch_experiment->verify_problems.clear();
						branch_experiment->verify_seeds.clear();
						branch_experiment->verify_original_scores.clear();
						branch_experiment->verify_branch_scores.clear();
						/**
						 * - simply rely on leaf experiment to verify
						 */
						#endif /* MDEBUG */

						branch_experiment->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
						branch_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
						branch_experiment->experiment_iter = 0;

						to_experiment = true;
					}
				}
				break;
			// case EXPERIMENT_TYPE_PASS_THROUGH:
			// 	{
			// 		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)this->verify_experiments.back();
			// 		if (pass_through_experiment->best_step_types.size() > 0) {
			// 			pass_through_experiment->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
			// 			pass_through_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			// 			pass_through_experiment->experiment_iter = 0;

			// 			to_experiment = true;
			// 		}
			// 	}
			// 	break;
			// case EXPERIMENT_TYPE_NEW_INFO:
			// 	{
			// 		double combined_branch_weight = 1.0;
			// 		AbstractExperiment* curr_experiment = this->verify_experiments.back();
			// 		while (true) {
			// 			if (curr_experiment == NULL) {
			// 				break;
			// 			}

			// 			switch (curr_experiment->type) {
			// 			case EXPERIMENT_TYPE_BRANCH:
			// 				{
			// 					BranchExperiment* branch_experiment = (BranchExperiment*)curr_experiment;
			// 					combined_branch_weight *= branch_experiment->branch_weight;
			// 				}
			// 				break;
			// 			case EXPERIMENT_TYPE_NEW_INFO:
			// 				{
			// 					NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)curr_experiment;
			// 					combined_branch_weight *= new_info_experiment->branch_weight;
			// 				}
			// 				break;
			// 			}
			// 			curr_experiment = curr_experiment->parent_experiment;
			// 		}

			// 		NewInfoExperiment* new_info_experiment = (NewInfoExperiment*)this->verify_experiments.back();
			// 		if (new_info_experiment->best_step_types.size() > 0
			// 				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
			// 			#if defined(MDEBUG) && MDEBUG
			// 			for (int p_index = 0; p_index < (int)new_info_experiment->verify_problems.size(); p_index++) {
			// 				delete new_info_experiment->verify_problems[p_index];
			// 			}
			// 			new_info_experiment->verify_problems.clear();
			// 			new_info_experiment->verify_seeds.clear();
			// 			new_info_experiment->verify_negative_scores.clear();
			// 			new_info_experiment->verify_positive_scores.clear();
			// 			#endif /* MDEBUG */

			// 			new_info_experiment->state = NEW_INFO_EXPERIMENT_STATE_EXPERIMENT;
			// 			new_info_experiment->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			// 			new_info_experiment->experiment_iter = 0;

			// 			to_experiment = true;
			// 		}
			// 	}
			// 	break;
			}

			if (!to_experiment) {
				AbstractExperiment* curr_experiment = this->verify_experiments.back()->parent_experiment;

				curr_experiment->experiment_iter++;
				int matching_index;
				for (int c_index = 0; c_index < (int)curr_experiment->child_experiments.size(); c_index++) {
					if (curr_experiment->child_experiments[c_index] == this->verify_experiments.back()) {
						matching_index = c_index;
						break;
					}
				}
				curr_experiment->child_experiments.erase(curr_experiment->child_experiments.begin() + matching_index);

				this->verify_experiments.back()->result = EXPERIMENT_RESULT_FAIL;
				this->verify_experiments.back()->finalize(NULL);
				delete this->verify_experiments.back();

				double target_count = (double)MAX_EXPERIMENT_NUM_EXPERIMENTS
					* pow(0.5, this->verify_experiments.size());
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						break;
					}

					if (curr_experiment->experiment_iter >= target_count) {
						AbstractExperiment* parent = curr_experiment->parent_experiment;

						parent->experiment_iter++;
						int matching_index;
						for (int c_index = 0; c_index < (int)parent->child_experiments.size(); c_index++) {
							if (parent->child_experiments[c_index] == curr_experiment) {
								matching_index = c_index;
								break;
							}
						}
						parent->child_experiments.erase(parent->child_experiments.begin() + matching_index);

						curr_experiment->result = EXPERIMENT_RESULT_FAIL;
						curr_experiment->finalize(NULL);
						delete curr_experiment;

						curr_experiment = parent;
						target_count *= 2.0;
					} else {
						break;
					}
				}
			}

			this->verify_experiments.clear();

			if (this->experiment_iter >= MAX_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			}
		}
	}
}
