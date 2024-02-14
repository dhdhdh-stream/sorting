#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void PassThroughExperiment::experiment_verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	PassThroughExperimentInstanceHistory* instance_history = new PassThroughExperimentInstanceHistory(this);
	history = instance_history;

	for (int s_index = 0; s_index < this->branch_experiment_step_index+1; s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			instance_history->pre_step_histories.push_back(action_node_history);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
			instance_history->pre_step_histories.push_back(scope_node_history);
			this->best_existing_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
			instance_history->pre_step_histories.push_back(scope_node_history);
			this->best_potential_scopes[s_index]->potential_activate(
				problem,
				context,
				run_helper,
				scope_node_history);
		}
	}

	if (!run_helper.exceeded_limit) {
		if (this->branch_experiment_step_index == (int)this->best_step_types.size()-1) {
			if (this->best_exit_depth == 0) {
				curr_node = this->best_exit_node;
			} else {
				exit_depth = this->best_exit_depth-1;
				exit_node = this->best_exit_node;
			}
		} else {
			if (this->best_step_types[this->branch_experiment_step_index+1] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[this->branch_experiment_step_index+1];
			} else if (this->best_step_types[this->branch_experiment_step_index+1] == STEP_TYPE_EXISTING_SCOPE) {
				curr_node = this->best_existing_scopes[this->branch_experiment_step_index+1];
			} else {
				curr_node = this->best_potential_scopes[this->branch_experiment_step_index+1];
			}
		}

		this->branch_experiment->parent_verify_activate(
			curr_node,
			problem,
			context,
			exit_depth,
			exit_node,
			run_helper,
			instance_history->branch_experiment_history);

		if (!run_helper.exceeded_limit
				&& exit_depth == -1) {
			map<AbstractNode*, int>::iterator it = this->node_to_step_index.find(curr_node);
			if (it != this->node_to_step_index.end()) {
				for (int s_index = it->second; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
						instance_history->post_step_histories.push_back(action_node_history);
						this->best_actions[s_index]->activate(
							curr_node,
							problem,
							context,
							exit_depth,
							exit_node,
							run_helper,
							action_node_history);
					} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
						ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_existing_scopes[s_index]);
						instance_history->post_step_histories.push_back(scope_node_history);
						this->best_existing_scopes[s_index]->potential_activate(
							problem,
							context,
							run_helper,
							scope_node_history);
					} else {
						ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_potential_scopes[s_index]);
						instance_history->post_step_histories.push_back(scope_node_history);
						this->best_potential_scopes[s_index]->potential_activate(
							problem,
							context,
							run_helper,
							scope_node_history);
					}
				}

				if (this->best_exit_depth == 0) {
					curr_node = this->best_exit_node;
				} else {
					exit_depth = this->best_exit_depth-1;
					exit_node = this->best_exit_node;
				}
			}
		}
	}
}

void PassThroughExperiment::experiment_verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->o_target_val_histories.push_back(target_val);

	if (this->state == PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_1ST_NEW
			&& (int)this->o_target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT_VERIFY_2ND_EXISTING;
			/**
			 * - leave this->state_iter unchanged
			 */
		} else {
			delete this->branch_experiment;
			this->branch_experiment = NULL;

			this->state_iter++;
			if (this->state_iter >= PASS_THROUGH_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				uniform_int_distribution<int> distribution(0, (int)this->best_step_types.size()-1);
				this->branch_experiment_step_index = distribution(generator);

				this->branch_experiment = new BranchExperiment(
					this->scope_context,
					this->node_context);
				if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_ACTION) {
					this->branch_experiment->node_context.back() = this->best_actions[this->branch_experiment_step_index];
				} else if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_EXISTING_SCOPE) {
					this->branch_experiment->node_context.back() = this->best_existing_scopes[this->branch_experiment_step_index];
				} else {
					this->branch_experiment->node_context.back() = this->best_potential_scopes[this->branch_experiment_step_index];
				}
				this->branch_experiment->parent_pass_through_experiment = this;
			}

			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
		}
	} else if ((int)this->o_target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		#if defined(MDEBUG) && MDEBUG
		this->o_target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->o_target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			cout << "PassThrough experiment success" << endl;
			cout << "this->scope_context:" << endl;
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				cout << c_index << ": " << this->scope_context[c_index]->id << endl;
			}
			cout << "this->node_context:" << endl;
			for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				cout << c_index << ": " << this->node_context[c_index]->id << endl;
			}
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
					cout << " E";
				} else {
					cout << " P";
				}
			}
			cout << endl;

			cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
			if (this->best_exit_node == NULL) {
				cout << "this->best_exit_node_id: " << -1 << endl;
			} else {
				cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
			}

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;
			cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			delete this->branch_experiment;
			this->branch_experiment = NULL;

			this->state_iter++;
			if (this->state_iter >= PASS_THROUGH_EXPERIMENT_NUM_EXPERIMENTS) {
				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				uniform_int_distribution<int> distribution(0, (int)this->best_step_types.size()-1);
				this->branch_experiment_step_index = distribution(generator);

				this->branch_experiment = new BranchExperiment(
					this->scope_context,
					this->node_context);
				if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_ACTION) {
					this->branch_experiment->node_context.back() = this->best_actions[this->branch_experiment_step_index];
				} else if (this->best_step_types[this->branch_experiment_step_index] == STEP_TYPE_EXISTING_SCOPE) {
					this->branch_experiment->node_context.back() = this->best_existing_scopes[this->branch_experiment_step_index];
				} else {
					this->branch_experiment->node_context.back() = this->best_potential_scopes[this->branch_experiment_step_index];
				}
				this->branch_experiment->parent_pass_through_experiment = this;
			}

			this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPERIMENT;
		}
	}
}
