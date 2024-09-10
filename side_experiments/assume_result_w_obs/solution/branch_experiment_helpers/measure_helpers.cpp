#include "branch_experiment.h"

#include <iostream>

#include "absolute_return_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	run_helper.num_actions++;

	history->instance_count++;

	run_helper.num_analyze += (int)this->input_types.size();

	vector<double> local_location = problem->get_location();

	vector<double> input_vals(this->input_types.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		switch (this->input_types[i_index]) {
		case INPUT_TYPE_GLOBAL:
			{
				vector<double> location = problem_type->relative_to_world(
					context.back().starting_location,
					this->input_locations[i_index]);

				map<vector<double>, double>::iterator it = run_helper.world_model.find(location);
				if (it != run_helper.world_model.end()) {
					input_vals[i_index] = it->second;
				}
			}
			break;
		case INPUT_TYPE_LOCAL:
			{
				vector<double> location = problem_type->relative_to_world(
					local_location,
					this->input_locations[i_index]);

				map<vector<double>, double>::iterator it = run_helper.world_model.find(location);
				if (it != run_helper.world_model.end()) {
					input_vals[i_index] = it->second;
				}
			}
			break;
		case INPUT_TYPE_HISTORY:
			{
				map<AbstractNode*, pair<vector<double>,vector<double>>>::iterator it
					= context.back().node_history.find(this->input_node_contexts[i_index]);
				if (it != context.back().node_history.end()) {
					input_vals[i_index] = it->second.second[this->input_obs_indexes[i_index]];
				}
			}
			break;
		}
	}
	this->network->activate(input_vals);

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = this->network->output->acti_vals[0] >= 0.0;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else if (this->best_step_types[0] == STEP_TYPE_SCOPE) {
				curr_node = this->best_scopes[0];
			} else if (this->best_step_types[0] == STEP_TYPE_RETURN) {
				curr_node = this->best_returns[0];
			} else {
				curr_node = this->best_absolute_returns[0];
			}
		}

		return true;
	}

	return false;
}

void BranchExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->result = EXPERIMENT_RESULT_FAIL;
	} else {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			double final_score = (target_val - run_helper.result) / history->instance_count;
			this->combined_score += final_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->combined_score > 0.0) {
			#endif /* MDEBUG */
				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index]->action.move;
					} else if (this->best_step_types[s_index] == STEP_TYPE_SCOPE) {
						cout << " E";
					} else {
						cout << " R";
					}
				}
				cout << endl;

				if (this->best_exit_next_node == NULL) {
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->combined_score: " << this->combined_score << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
