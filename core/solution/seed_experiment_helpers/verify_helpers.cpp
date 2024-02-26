#include "seed_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "seed_experiment_filter.h"
#include "solution.h"

using namespace std;

void SeedExperiment::verify_backprop(double target_val) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state == SEED_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints));

		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			this->combined_score = 0.0;

			this->o_target_val_histories.reserve(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

			cout << "SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING" << endl;
			this->state = SEED_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			cout << "SEED_EXPERIMENT_STATE_FIND_GATHER" << endl;
			this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
			this->state_iter = 0;
			this->sub_state_iter = -1;
		}
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->combined_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		double combined_improvement = this->combined_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (this->existing_score_standard_deviation / sqrt(VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints));

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (combined_improvement_t_score > 1.645) {	// >95%
		#endif /* MDEBUG */
			cout << "Seed" << endl;
			cout << "verify" << endl;
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

			cout << "this->combined_score: " << this->combined_score << endl;
			cout << "this->existing_average_score: " << this->existing_average_score << endl;
			cout << "this->existing_score_standard_deviation: " << this->existing_score_standard_deviation << endl;
			cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

			cout << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			cout << "SEED_EXPERIMENT_STATE_FIND_GATHER" << endl;
			this->state = SEED_EXPERIMENT_STATE_FIND_GATHER;
			this->state_iter = 0;
			this->sub_state_iter = -1;
		}
	}
}
