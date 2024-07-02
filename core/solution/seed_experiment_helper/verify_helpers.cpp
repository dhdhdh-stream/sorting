#include "seed_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_set.h"

using namespace std;

void SeedExperiment::verify_activate(AbstractNode*& curr_node,
									 Problem* problem,
									 vector<ContextLayer>& context,
									 RunHelper& run_helper,
									 SeedExperimentHistory* history) {
	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1, 0.0));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;
			Scope* scope = (Scope*)scope_history->scope;

			if (scope->eval_network != NULL) {
				scope_history->callback_experiment_history = history;
				scope_history->callback_experiment_indexes.push_back(
					(int)history->predicted_scores.size()-1);
				scope_history->callback_experiment_layers.push_back(l_index);
			}
		}
		break;
	}

	if (this->is_pass_through) {
		for (int s_index = 0; s_index < (int)this->best_seed_step_types.size(); s_index++) {
			if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_seed_actions[s_index]->explore_activate(
					problem,
					context.back().scope_history->node_histories);
			} else {
				this->best_seed_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper,
					context.back().scope_history->node_histories);
			}
		}

		curr_node = this->best_seed_exit_next_node;
	} else {
		for (int s_index = 0; s_index < this->branch_index; s_index++) {
			if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_seed_actions[s_index]->explore_activate(
					problem,
					context.back().scope_history->node_histories);
			} else {
				this->best_seed_scopes[s_index]->explore_activate(
					problem,
					context,
					run_helper,
					context.back().scope_history->node_histories);
			}
		}

		run_helper.num_decisions++;

		vector<double> new_input_vals(this->new_input_scope_contexts.size(), 0.0);
		for (int i_index = 0; i_index < (int)this->new_input_scope_contexts.size(); i_index++) {
			int curr_layer = 0;
			AbstractScopeHistory* curr_scope_history = context.back().scope_history;
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
								new_input_vals[i_index] = branch_node_history->score;
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
		double new_predicted_score = this->new_network->output->acti_vals[0];

		if (new_predicted_score < 0.0) {
			for (int s_index = 0; s_index < (int)this->best_back_step_types.size(); s_index++) {
				if (this->best_back_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_back_actions[s_index]->action);
				} else {
					this->best_back_scopes[s_index]->explore_activate(
						problem,
						context,
						run_helper);
				}

				if (run_helper.exceeded_limit) {
					break;
				}
			}

			curr_node = this->best_back_exit_next_node;
		} else {
			for (int s_index = this->branch_index; s_index < (int)this->best_seed_step_types.size(); s_index++) {
				if (this->best_seed_step_types[s_index] == STEP_TYPE_ACTION) {
					this->best_seed_actions[s_index]->explore_activate(
						problem,
						context.back().scope_history->node_histories);
				} else {
					this->best_seed_scopes[s_index]->explore_activate(
						problem,
						context,
						run_helper,
						context.back().scope_history->node_histories);
				}
			}

			curr_node = this->best_seed_exit_next_node;
		}
	}
}

void SeedExperiment::verify_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history) / (int)scope_history->callback_experiment_indexes.size();
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void SeedExperiment::verify_backprop(
		double target_val,
		RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		SeedExperimentHistory* history = (SeedExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double final_score;
			switch (this->score_type) {
			case SCORE_TYPE_TRUTH:
				final_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
				break;
			case SCORE_TYPE_ALL:
				{
					double sum_score = (target_val - solution_set->average_score) / (int)history->predicted_scores.size();
					for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
						sum_score += history->predicted_scores[i_index][l_index];
					}
					final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
				}
				break;
			}

			this->combined_score += final_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= VERIFY_NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->combined_score > this->existing_average_score) {
			#endif /* MDEBUG */
				cout << "SeedExperiment verify" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;

				cout << "this->combined_score: " << this->combined_score << endl;
				cout << "this->existing_average_score: " << this->existing_average_score << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				if (this->is_pass_through) {
					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
					this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

					this->state = SEED_EXPERIMENT_STATE_CAPTURE_VERIFY;
					this->state_iter = 0;
				}
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
