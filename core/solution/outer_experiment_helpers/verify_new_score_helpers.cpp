#include "outer_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void OuterExperiment::verify_new_score_activate(
		Problem* problem,
		RunHelper& run_helper) {
	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = NULL;
	context.back().node = NULL;

	// unused
	AbstractNode* curr_node = NULL;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

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
		} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_potential_scopes[s_index]);
			this->best_potential_scopes[s_index]->activate(problem,
														   context,
														   run_helper,
														   potential_scope_node_history);
			delete potential_scope_node_history;
		} else if (this->best_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
			PotentialScopeNodeHistory* potential_scope_node_history = new PotentialScopeNodeHistory(this->best_existing_scopes[s_index]);
			this->best_existing_scopes[s_index]->activate(problem,
														  context,
														  run_helper,
														  potential_scope_node_history);
			delete potential_scope_node_history;
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_root_scope_nodes[s_index]);
			this->best_root_scope_nodes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}
}

void OuterExperiment::verify_new_score_backprop(double target_val) {
	this->target_val_histories.push_back(target_val);

	if (this->state == OUTER_EXPERIMENT_STATE_VERIFY_1ST_NEW_SCORE
			&& (int)this->target_val_histories.size() >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		#if defined(MDEBUG) && MDEBUG
		this->target_val_histories.clear();

		if (rand()%2 == 0) {
		#else
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		this->target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			this->state = OUTER_EXPERIMENT_STATE_VERIFY_2ND_EXISTING_SCORE;
			this->state_iter = 0;
		} else {
			cout << "Outer verify 1st fail" << endl;
			this->state = OUTER_EXPERIMENT_STATE_FAIL;
		}
	} else if ((int)this->target_val_histories.size() >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints; d_index++) {
			sum_scores += this->target_val_histories[d_index];
		}
		double new_average_score = sum_scores / (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		this->target_val_histories.clear();

		double score_improvement = new_average_score - this->existing_average_score;
		double score_standard_deviation = sqrt(this->existing_score_variance);
		double score_improvement_t_score = score_improvement
			/ (score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (score_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			cout << "Outer" << endl;
			cout << "verify" << endl;
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->best_actions[s_index]->action.move;
				} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					cout << " S";
				} else {
					cout << " R";
				}
			}
			cout << endl;

			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "new_average_score: " << new_average_score << endl;
			cout << "score_improvement_t_score: " << score_improvement_t_score << endl;

			cout << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = OUTER_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			finalize();
			#endif /* MDEBUG */
		} else {
			cout << "Outer verify 2nd fail" << endl;
			this->state = OUTER_EXPERIMENT_STATE_FAIL;
		}
	}
}
