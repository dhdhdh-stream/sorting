#include "new_info_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

bool NewInfoExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		NewInfoExperimentHistory* history) {
	run_helper.num_decisions++;

	AbstractScopeHistory* scope_history;
	this->new_info_scope->explore_activate(problem,
										   context,
										   run_helper,
										   scope_history);

	switch (this->score_type) {
	case SCORE_TYPE_TRUTH:
		history->predicted_scores.push_back(vector<double>());
		break;
	case SCORE_TYPE_ALL:
		history->predicted_scores.push_back(vector<double>(context.size()-1));
		for (int l_index = 0; l_index < (int)context.size()-1; l_index++) {
			ScopeHistory* scope_history = (ScopeHistory*)context[l_index].scope_history;

			scope_history->callback_experiment_history = history;
			scope_history->callback_experiment_indexes.push_back(
				(int)history->predicted_scores.size()-1);
			scope_history->callback_experiment_layers.push_back(l_index);
		}
		break;
	}

	vector<double> new_input_vals(this->new_input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->new_input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_history->node_histories.find(
			this->new_input_node_contexts[i_index]);
		if (it != scope_history->node_histories.end()) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
			new_input_vals[i_index] = action_node_history->obs_snapshot[this->new_input_obs_indexes[i_index]];
		}
	}
	this->new_network->activate(new_input_vals);
	#if defined(MDEBUG) && MDEBUG
	#else
	double new_predicted_score = this->new_network->output->acti_vals[0];
	#endif /* MDEBUG */

	delete scope_history;

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = new_predicted_score >= 0.0;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		this->branch_count++;

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
		this->original_count++;

		return false;
	}
}

void NewInfoExperiment::measure_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

	ScopeHistory* scope_history = (ScopeHistory*)context.back().scope_history;

	double predicted_score;
	if (run_helper.exceeded_limit) {
		predicted_score = -1.0;
	} else {
		predicted_score = calc_score(scope_history);
	}
	for (int i_index = 0; i_index < (int)scope_history->callback_experiment_indexes.size(); i_index++) {
		history->predicted_scores[scope_history->callback_experiment_indexes[i_index]]
			[scope_history->callback_experiment_layers[i_index]] = predicted_score;
	}
}

void NewInfoExperiment::measure_backprop(double target_val,
										 RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
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

			this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else {
		NewInfoExperimentHistory* history = (NewInfoExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double final_score;
			switch (this->score_type) {
			case SCORE_TYPE_TRUTH:
				final_score = target_val - solution_set->average_score;
				break;
			case SCORE_TYPE_ALL:
				{
					double sum_score = target_val - solution_set->average_score;
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
		if (this->sub_state_iter >= NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			this->branch_weight = (double)this->branch_count / (double)(this->original_count + this->branch_count);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->branch_weight > 0.01
					&& this->combined_score >= this->existing_average_score) {
			#endif /* MDEBUG */
				if (this->branch_weight > PASS_THROUGH_BRANCH_WEIGHT
						&& this->new_average_score >= this->existing_average_score) {
					this->is_pass_through = true;

					this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

					this->state = NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING;
					this->state_iter = 0;
				} else {
					this->is_pass_through = false;

					this->branch_node = new InfoBranchNode();
					this->branch_node->parent = this->scope_context;
					this->branch_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					Solution* solution = solution_set->solutions[solution_set->curr_solution_index];
					if (solution->info_scopes.size() > 0) {
						this->existing_info_scope_index = 0;
						this->existing_is_negate = false;

						this->combined_score = 0.0;

						this->state = NEW_INFO_EXPERIMENT_STATE_TRY_EXISTING_INFO;
						this->state_iter = 0;
						this->sub_state_iter = 0;
					} else {
						this->use_existing = false;

						this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

						this->state = NEW_INFO_EXPERIMENT_STATE_VERIFY_EXISTING;
						this->state_iter = 0;
					}
				}
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

					this->state = NEW_INFO_EXPERIMENT_STATE_EXPLORE_SEQUENCE;
					this->state_iter = 0;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		}
	}
}
