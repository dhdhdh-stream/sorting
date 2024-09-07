#include "info_pass_through_experiment.h"

#include "action_node.h"
#include "constants.h"
#include "eval_helpers.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int INITIAL_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 2;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 2;
const int EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int INITIAL_NUM_TRUTH_PER_ITER = 5;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_1ST_NUM_TRUTH_PER_ITER = 25;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int VERIFY_2ND_NUM_TRUTH_PER_ITER = 100;
const int EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void InfoPassThroughExperiment::explore_activate(
		AbstractNode*& curr_node,
		Problem* problem) {
	for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
		problem->perform_action(this->actions[s_index]->action);
	}

	curr_node = this->exit_next_node;
}

void InfoPassThroughExperiment::explore_info_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

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
}

void InfoPassThroughExperiment::explore_back_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

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

void InfoPassThroughExperiment::explore_backprop(
		double target_val,
		RunHelper& run_helper) {
	bool is_fail = false;

	if (run_helper.exceeded_limit) {
		is_fail = true;
	} else {
		InfoPassThroughExperimentHistory* history = (InfoPassThroughExperimentHistory*)run_helper.experiment_histories.back();

		this->state_iter++;
		if (this->state_iter == INITIAL_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= INITIAL_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_1ST_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (false) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		} else if (this->state_iter == VERIFY_2ND_NUM_TRUTH_PER_ITER
				&& this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->info_score < 0.0) {
			#endif /* MDEBUG */
				is_fail = true;
			}
		}

		for (int i_index = 0; i_index < (int)history->predicted_scores.size(); i_index++) {
			double final_score;
			switch (this->score_type) {
			case SCORE_TYPE_TRUTH:
				final_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
				break;
			case SCORE_TYPE_ALL:
				{
					double sum_score = (target_val - solution->average_score) / (int)history->predicted_scores.size();
					for (int l_index = 0; l_index < (int)history->predicted_scores[i_index].size(); l_index++) {
						sum_score += history->predicted_scores[i_index][l_index];
					}
					final_score = sum_score / ((int)history->predicted_scores[i_index].size() + 1);
				}
				break;
			}

			this->info_score += final_score - this->existing_average_score;
			this->sub_state_iter++;

			if (this->sub_state_iter == INITIAL_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= INITIAL_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_1ST_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_1ST_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (false) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			} else if (this->sub_state_iter == VERIFY_2ND_NUM_SAMPLES_PER_ITER
					&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
				#if defined(MDEBUG) && MDEBUG
				if (rand()%2 == 0) {
				#else
				if (this->info_score < 0.0) {
				#endif /* MDEBUG */
					is_fail = true;
				}
			}
		}
	}

	if (is_fail) {
		for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
			delete this->actions[s_index];
		}
		this->actions.clear();

		this->explore_iter++;
		if (this->explore_iter >= EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			vector<AbstractNode*> possible_exits;

			if (((ActionNode*)this->node_context)->next_node == NULL) {
				possible_exits.push_back(NULL);
			}

			AbstractNode* starting_node = ((ActionNode*)this->node_context)->next_node;
			InfoScope* parent_scope = (InfoScope*)this->scope_context;
			parent_scope->random_exit_activate(
				starting_node,
				possible_exits);

			uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
			int random_index = distribution(generator);
			this->exit_next_node = possible_exits[random_index];

			int new_num_steps;
			uniform_int_distribution<int> uniform_distribution(0, 1);
			geometric_distribution<int> geometric_distribution(0.5);
			if (random_index == 0) {
				new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);
			} else {
				new_num_steps = uniform_distribution(generator) + geometric_distribution(generator);
			}

			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				this->actions.push_back(new_action_node);
			}

			this->info_score = 0.0;

			this->state_iter = 0;
			this->sub_state_iter = 0;
		}
	} else if (this->sub_state_iter >= VERIFY_2ND_NUM_SAMPLES_PER_ITER
			&& this->state_iter >= VERIFY_2ND_NUM_TRUTH_PER_ITER) {
		for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
			this->actions[s_index]->parent = this->scope_context;
			this->actions[s_index]->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;
		}

		int exit_node_id;
		AbstractNode* exit_node;
		if (this->exit_next_node == NULL) {
			ActionNode* new_ending_node = new ActionNode();
			new_ending_node->parent = this->scope_context;
			new_ending_node->id = this->scope_context->node_counter;
			this->scope_context->node_counter++;

			new_ending_node->action = Action(ACTION_NOOP);

			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			this->ending_node = new_ending_node;

			exit_node_id = new_ending_node->id;
			exit_node = new_ending_node;
		} else {
			exit_node_id = this->exit_next_node->id;
			exit_node = this->exit_next_node;
		}

		for (int s_index = 0; s_index < (int)this->actions.size(); s_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (s_index == (int)this->actions.size()-1) {
				next_node_id = exit_node_id;
				next_node = exit_node;
			} else {
				next_node_id = this->actions[s_index+1]->id;
				next_node = this->actions[s_index+1];
			}

			this->actions[s_index]->next_node_id = next_node_id;
			this->actions[s_index]->next_node = next_node;
		}

		InfoScope* parent_scope = (InfoScope*)this->scope_context;
		this->new_input_node_contexts = parent_scope->input_node_contexts;
		this->new_input_obs_indexes = parent_scope->input_obs_indexes;
		this->new_network = new Network(parent_scope->network);

		this->scope_histories.reserve(NUM_DATAPOINTS);
		this->target_val_histories.reserve(NUM_DATAPOINTS);

		uniform_int_distribution<int> until_distribution(0, (int)this->average_instances_per_run-1.0);
		this->num_instances_until_target = 1 + until_distribution(generator);

		this->state = INFO_PASS_THROUGH_EXPERIMENT_STATE_TRAIN_EXISTING;
		this->state_iter = 0;
	}
}
