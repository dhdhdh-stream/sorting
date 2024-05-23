#include "orientation_experiment.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
#endif /* MDEBUG */

void OrientationExperiment::explore_impact_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	this->scope_histories.push_back(new ScopeHistory(context.back().scope_history));

	for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
		if (this->step_types[s_index] == STEP_TYPE_ACTION) {
			problem->perform_action(this->actions[s_index]->action);
		} else {
			this->scopes[s_index]->explore_activate(
				problem,
				run_helper);
		}
	}

	curr_node = this->exit_next_node;

	run_helper.num_actions_limit = MAX_NUM_ACTIONS_LIMIT_MULTIPLIER * solution->explore_scope_max_num_actions;
}

void OrientationExperiment::explore_impact_backprop(
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (run_helper.num_actions_limit > 0) {
		this->eval_context->activate_end(problem,
										 run_helper,
										 eval_history);

		double target_impact;
		if (context.size() == 1) {
			target_impact = problem->score_result(run_helper.num_decisions);
		} else {
			context[context.size()-2].scope->eval->activate_end(
				problem,
				run_helper,
				history->outer_eval_history);
			target_impact = context[context.size()-2].scope->eval->calc_impact(
				history->outer_eval_history);
		}

		this->target_val_histories.push_back(target_impact);

		this->state_iter++;
		if (this->state_iter == INITIAL_NUM_SAMPLES_PER_ITER) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double sum_impacts = 0.0;
			for (int i_index = 0; i_index < INITIAL_NUM_SAMPLES_PER_ITER; i_index++) {
				sum_impacts += this->target_val_histories[i_index];
			}
			double average_new_impact = sum_impacts / INITIAL_NUM_SAMPLES_PER_ITER;

			if (average_new_impact < 0.0) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->actions[s_index];
					} else {
						delete this->scopes[s_index];
					}
				}

				this->step_types.clear();
				this->actions.clear();
				this->scopes.clear();

				for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
					delete this->scope_histories[h_index];
				}
				this->scope_histories.clear();
				this->target_val_histories.clear();

				this->explore_iter++;
				if (this->explore_iter >= EXPLORE_ITERS) {
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					this->state = ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS;
				}
			}
		} else if (this->state_iter >= NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			double sum_impacts = 0.0;
			for (int i_index = 0; i_index < INITIAL_NUM_SAMPLES_PER_ITER; i_index++) {
				sum_impacts += this->target_val_histories[i_index];
			}
			double average_new_impact = sum_impacts / INITIAL_NUM_SAMPLES_PER_ITER;

			if (average_new_impact >= 0.0) {
			#endif /* MDEBUG */
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						this->actions[s_index]->parent = this->scope_context;
						this->actions[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					} else {
						this->scopes[s_index]->parent = this->scope_context;
						this->scopes[s_index]->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;
					}
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

				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					int next_node_id;
					AbstractNode* next_node;
					if (s_index == (int)this->step_types.size()-1) {
						next_node_id = exit_node_id;
						next_node = exit_node;
					} else {
						if (this->step_types[s_index+1] == STEP_TYPE_ACTION) {
							next_node_id = this->actions[s_index+1]->id;
							next_node = this->actions[s_index+1];
						} else {
							next_node_id = this->scopes[s_index+1]->id;
							next_node = this->scopes[s_index+1];
						}
					}

					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						this->actions[s_index]->next_node_id = next_node_id;
						this->actions[s_index]->next_node = next_node;
					} else if (this->step_types[s_index] == STEP_TYPE_SCOPE) {
						this->scopes[s_index]->next_node_id = next_node_id;
						this->scopes[s_index]->next_node = next_node;
					}
				}

				train_new();

				this->state = ORIENTATION_EXPERIMENT_STATE_MEASURE;
				this->state_iter = 0;
			} else {
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						delete this->actions[s_index];
					} else {
						delete this->scopes[s_index];
					}
				}

				this->step_types.clear();
				this->actions.clear();
				this->scopes.clear();

				for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
					delete this->scope_histories[h_index];
				}
				this->scope_histories.clear();
				this->target_val_histories.clear();

				this->explore_iter++;
				if (this->explore_iter >= EXPLORE_ITERS) {
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					this->state = ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS;
				}
			}
		}
	} else {
		for (int s_index = 0; s_index < (int)this->curr_step_types.size(); s_index++) {
			if (this->curr_step_types[s_index] == STEP_TYPE_ACTION) {
				delete this->curr_actions[s_index];
			} else {
				delete this->curr_scopes[s_index];
			}
		}

		this->curr_step_types.clear();
		this->curr_actions.clear();
		this->curr_scopes.clear();

		for (int h_index = 0; h_index < (int)this->scope_histories.size(); h_index++) {
			delete this->scope_histories[h_index];
		}
		this->scope_histories.clear();
		this->target_val_histories.clear();

		this->explore_iter++;
		if (this->explore_iter >= EXPLORE_ITERS) {
			this->result = EXPERIMENT_RESULT_FAIL;
		} else {
			this->state = ORIENTATION_EXPERIMENT_STATE_EXPLORE_MISGUESS;
		}
	}

	run_helper.num_actions_limit = -1;
}
