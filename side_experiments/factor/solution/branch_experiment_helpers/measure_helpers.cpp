#include "branch_experiment.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 10;
#else
const int MEASURE_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

void BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										ScopeHistory* scope_history,
										BranchExperimentHistory* history) {
	run_helper.has_explore = true;

	map<pair<int,int>, double> factors;
	gather_factors(run_helper,
				   scope_history,
				   factors);

	double sum_vals = 0.0;
	for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
		map<pair<int,int>, double>::iterator it = factors.find(this->new_factor_ids[f_index]);
		if (it != factors.end()) {
			sum_vals += this->new_factor_weights[f_index] * it->second;
		}
	}

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = sum_vals >= 0.0;
	#endif /* MDEBUG */

	if (decision_is_branch) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				context.back().node_id = -1;

				this->best_scopes[s_index]->activate(problem,
					context,
					run_helper);
			}

			run_helper.num_actions += 2;
		}
	}
}

void BranchExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	this->combined_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		this->combined_score /= this->state_iter;

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
					cout << " " << this->best_actions[s_index].move;
				} else {
					cout << " E" << this->best_scopes[s_index]->id;
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
