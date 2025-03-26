#include "commit_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void CommitExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	run_helper.has_explore = true;

	for (int n_index = 0; n_index < this->step_iter; n_index++) {
		switch (this->new_nodes[n_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[n_index];
				node->commit_activate(problem,
									  run_helper,
									  scope_history);
			}
			break;
		}
	}

	run_helper.num_actions++;

	double sum_vals = this->commit_new_average_score;
	for (int f_index = 0; f_index < (int)this->commit_new_factor_ids.size(); f_index++) {
		double val;
		fetch_factor_helper(run_helper,
							scope_history,
							this->commit_new_factor_ids[f_index],
							val);
		sum_vals += this->commit_new_factor_weights[f_index] * val;
	}

	bool decision_is_branch;
	if (sum_vals >= 0.0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}

	if (decision_is_branch) {
		for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
			if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->save_actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
				this->save_scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->save_exit_next_node;
	} else {
		for (int n_index = this->step_iter; n_index < (int)this->new_nodes.size(); n_index++) {
			switch (this->new_nodes[n_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			}
		}

		curr_node = this->best_exit_next_node;
	}
}

void CommitExperiment::measure_backprop(double target_val,
										RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		double new_score = this->combined_score / this->state_iter;
		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		if (new_score > this->o_existing_average_score) {
		#endif /* MDEBUG */
			this->improvement = new_score - this->o_existing_average_score;

			cout << "CommitExperiment success" << endl;

			cout << "this->o_existing_average_score: " << this->o_existing_average_score << endl;
			cout << "this->commit_existing_average_score: " << this->commit_existing_average_score << endl;
			cout << "this->commit_new_average_score: " << this->commit_new_average_score << endl;

			cout << "this->new_nodes.size(): " << this->new_nodes.size() << endl;
			cout << "this->step_iter: " << this->step_iter << endl;

			#if defined(MDEBUG) && MDEBUG
			this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
			this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

			this->state = COMMIT_EXPERIMENT_STATE_CAPTURE_VERIFY;
			this->state_iter = 0;
			#else
			this->result = EXPERIMENT_RESULT_SUCCESS;
			#endif /* MDEBUG */
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
