#include "pass_through_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_set.h"

using namespace std;

void PassThroughExperiment::verify_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	history->instance_count++;

	if (this->best_step_types.size() == 0) {
		curr_node = this->best_exit_next_node;
	} else {
		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			curr_node = this->best_actions[0];
		} else {
			curr_node = this->best_scopes[0];
		}
	}
}

void PassThroughExperiment::verify_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		double final_score = (target_val - solution_set->average_score) / history->instance_count;
		this->target_val_histories.push_back(final_score);
	}

	this->state_iter++;
	if ((int)this->target_val_histories.size() >= VERIFY_NUM_DATAPOINTS
			&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
		int num_instances = (int)this->target_val_histories.size();

		double sum_scores = 0.0;
		for (int d_index = 0; d_index < num_instances; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / num_instances;

		this->target_val_histories.clear();

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_average_score > this->existing_average_score) {
		#endif /* MDEBUG */
			cout << "PassThrough" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
					cout << " E";
				}
			}
			cout << endl;

			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
