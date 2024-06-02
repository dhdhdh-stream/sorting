#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "new_info_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::verify_activate(AbstractNode*& curr_node,
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

		if (decision_is_branch) {
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
			return false;
		}
	}
}

void BranchExperiment::verify_back_activate(
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

void BranchExperiment::verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	double final_normalized_score = (target_val - solution->average_score) / solution->score_standard_deviation;
	for (int i_index = 0; i_index < (int)history->starting_predicted_scores.size(); i_index++) {
		double sum_score = 0.0;
		for (int l_index = 0; l_index < (int)history->starting_predicted_scores[i_index].size(); l_index++) {
			sum_score += history->normalized_scores[i_index][l_index];
		}
		sum_score += final_normalized_score;
		this->combined_score += sum_score;
		this->state_iter++;
	}

	if (this->state_iter >= VERIFY_NUM_DATAPOINTS) {
		this->combined_score /= this->state_iter;

		double combined_branch_weight = 1.0;
		AbstractExperiment* curr_experiment = this;
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

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (this->combined_score > this->existing_average_score) {
		#endif /* MDEBUG */
			cout << "verify" << endl;
			cout << "this->parent_experiment: " << this->parent_experiment << endl;
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

			cout << "this->branch_weight: " << this->branch_weight << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			if (this->is_pass_through) {
				if (this->parent_experiment == NULL) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					vector<AbstractExperiment*> verify_experiments;
					verify_experiments.insert(verify_experiments.begin(), this);
					AbstractExperiment* curr_experiment = this->parent_experiment;
					while (true) {
						if (curr_experiment->parent_experiment == NULL) {
							/**
							 * - don't include root
							 */
							break;
						} else {
							verify_experiments.insert(verify_experiments.begin(), curr_experiment);
							curr_experiment = curr_experiment->parent_experiment;
						}
					}

					this->root_experiment->verify_experiments = verify_experiments;

					this->root_experiment->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

					this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_EXISTING;

					this->state = BRANCH_EXPERIMENT_STATE_ROOT_VERIFY;
				}
			} else {
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
			}
			#else
			if (this->parent_experiment == NULL) {
				this->result = EXPERIMENT_RESULT_SUCCESS;
			} else {
				vector<AbstractExperiment*> verify_experiments;
				verify_experiments.insert(verify_experiments.begin(), this);
				AbstractExperiment* curr_experiment = this->parent_experiment;
				while (true) {
					if (curr_experiment->parent_experiment == NULL) {
						/**
						 * - don't include root
						 */
						break;
					} else {
						verify_experiments.insert(verify_experiments.begin(), curr_experiment);
						curr_experiment = curr_experiment->parent_experiment;
					}
				}

				this->root_experiment->verify_experiments = verify_experiments;

				this->root_experiment->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

				this->root_experiment->root_state = ROOT_EXPERIMENT_STATE_VERIFY_EXISTING;

				this->state = BRANCH_EXPERIMENT_STATE_ROOT_VERIFY;
			}
			#endif /* MDEBUG */
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0
				&& rand()%4 == 0) {
		#else
		} else if (this->best_step_types.size() > 0
				&& combined_branch_weight > EXPERIMENT_COMBINED_MIN_BRANCH_WEIGHT) {
		#endif /* MDEBUG */
			this->state = BRANCH_EXPERIMENT_STATE_EXPERIMENT;
			this->root_state = ROOT_EXPERIMENT_STATE_EXPERIMENT;
			this->experiment_iter = 0;
		} else {
			this->explore_iter++;
			if (this->explore_iter < MAX_EXPLORE_TRIES) {
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->best_actions[s_index];
					} else {
						delete this->best_scopes[s_index];
					}
				}

				this->best_step_types.clear();
				this->best_actions.clear();
				this->best_scopes.clear();

				if (this->ending_node != NULL) {
					delete this->ending_node;
					this->ending_node = NULL;
				}
				if (this->branch_node != NULL) {
					delete this->branch_node;
					this->branch_node = NULL;
				}

				this->new_input_scope_contexts.clear();
				this->new_input_node_contexts.clear();
				this->new_input_obs_indexes.clear();
				if (this->new_network != NULL) {
					delete this->new_network;
					this->new_network = NULL;
				}

				uniform_int_distribution<int> neutral_distribution(0, 9);
				if (neutral_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_NEUTRAL;
				} else {
					uniform_int_distribution<int> best_distribution(0, 1);
					if (best_distribution(generator) == 0) {
						this->explore_type = EXPLORE_TYPE_BEST;

						this->best_surprise = 0.0;
					} else {
						this->explore_type = EXPLORE_TYPE_GOOD;
					}
				}

				this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
				this->state_iter = 0;
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
