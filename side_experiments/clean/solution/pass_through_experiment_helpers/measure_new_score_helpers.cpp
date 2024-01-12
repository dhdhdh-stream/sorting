#include "pass_through_experiment.h"

#include <iostream>
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "state_network.h"
#include "globals.h"
#include "solution_helpers.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "state.h"

using namespace std;

void PassThroughExperiment::measure_new_score_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper,
		AbstractExperimentHistory*& history) {
	PassThroughExperimentInstanceHistory* instance_history = new PassThroughExperimentInstanceHistory(this);
	history = instance_history;

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
			this->best_potential_scopes[s_index]->activate(problem,
														   context,
														   run_helper,
														   potential_scope_node_history);
			delete potential_scope_node_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node = this->best_exit_node;
	} else {
		curr_node = NULL;

		exit_depth = this->best_exit_depth-1;
		exit_node = this->best_exit_node;
	}
}

void PassThroughExperiment::measure_new_score_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	if ((int)this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->new_average_score = sum_scores / solution->curr_num_datapoints;

		this->o_target_val_histories.clear();

		// this->best_try_instance->result = this->new_average_score;
		// ofstream output_file;
		// output_file.open(path + "saves/tries/" + to_string(time(NULL)) + "_t.txt");
		// this->best_try_instance->save(output_file);
		// output_file.close();
		// Scope* parent_scope = solution->scopes[this->scope_context[0]];
		// parent_scope->tries->update(this->best_try_instance);
		// this->best_try_instance = NULL;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%4 == 0) {
		#else
		double score_improvement = this->new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(solution->curr_num_datapoints));
		
		// cout << "PassThrough" << endl;
		// cout << "measure" << endl;
		// cout << "this->scope_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->scope_context[c_index] << endl;
		// }
		// cout << "this->node_context:" << endl;
		// for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
		// 	cout << c_index << ": " << this->node_context[c_index] << endl;
		// }
		// cout << "new explore path:";
		// for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// 	if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
		// 		cout << " " << this->best_actions[s_index]->action.move;
		// 	} else {
		// 		cout << " S";
		// 	}
		// }
		// cout << endl;

		// cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
		// if (this->best_exit_node == NULL) {
		// 	cout << "this->best_exit_node_id: " << -1 << endl;
		// } else {
		// 	cout << "this->best_exit_node_id: " << this->best_exit_node->id << endl;
		// }

		// cout << "this->new_average_score: " << this->new_average_score << endl;
		// cout << "this->existing_average_score: " << this->existing_average_score << endl;
		// cout << "score_standard_deviation: " << score_standard_deviation << endl;
		// cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

		// cout << endl;

		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					clean_state(this->best_potential_scopes[s_index]);
				}
			}

			this->o_target_val_histories.reserve(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST_EXISTING_SCORE;
			this->state_iter = 0;
		#if defined(MDEBUG) && MDEBUG
		} else if (this->best_step_types.size() > 0 && rand()%2 == 0) {
		#else
		} else if (this->best_step_types.size() > 0 && score_improvement_t_score > -0.674) {	// <75%
		#endif /* MDEBUG */
			/**
			 * - TODO: consider other conditions
			 *   - e.g., small number of particularly high scoring instances
			 */

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					clean_state(this->best_potential_scopes[s_index]);
				}
			}

			// reserve at least solution->curr_num_datapoints
			this->i_misguess_histories.reserve(solution->curr_num_datapoints);

			this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_MISGUESS;
			this->state_iter = 0;
		} else {
			this->state = PASS_THROUGH_EXPERIMENT_STATE_FAIL;
		}
	}
}
