#include "eval_pass_through_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "eval.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"

using namespace std;

void EvalPassThroughExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper) {
	if (this->info_scope == NULL) {
		if (this->step_types.size() == 0) {
			curr_node = this->exit_next_node;
		} else {
			if (this->step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->actions[0];
			} else {
				curr_node = this->scopes[0];
			}
		}
	} else {
		ScopeHistory* inner_scope_history;
		bool inner_is_positive;
		this->info_scope->activate(problem,
								   run_helper,
								   inner_scope_history,
								   inner_is_positive);

		delete inner_scope_history;

		if ((this->is_negate && !inner_is_positive)
				|| (!this->is_negate && inner_is_positive)) {
			if (this->step_types.size() == 0) {
				curr_node = this->exit_next_node;
			} else {
				if (this->step_types[0] == STEP_TYPE_ACTION) {
					curr_node = this->actions[0];
				} else {
					curr_node = this->scopes[0];
				}
			}
		}
	}
}

void EvalPassThroughExperiment::measure_backprop(
		EvalHistory* outer_eval_history,
		EvalHistory* eval_history,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	double target_impact;
	if (context.size() == 1) {
		target_impact = problem->score_result(run_helper.num_decisions);
	} else {
		target_impact = context[context.size()-2].scope->eval->calc_impact(outer_eval_history);
	}

	vector<double> input_vals(this->input_node_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_node_contexts.size(); i_index++) {
		map<AbstractNode*, AbstractNodeHistory*>::iterator it = eval_history->scope_history->node_histories.find(
			this->input_node_contexts[i_index]);
		if (it != eval_history->scope_history->node_histories.end()) {
			switch (this->input_node_contexts[i_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
					input_vals[i_index] = action_node_history->obs_snapshot[this->input_obs_indexes[i_index]];
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNodeHistory* branch_node_history = (BranchNodeHistory*)it->second;
					if (branch_node_history->is_branch) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_SCOPE:
				{
					InfoScopeNodeHistory* info_scope_node_history = (InfoScopeNodeHistory*)it->second;
					if (info_scope_node_history->is_positive) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			case NODE_TYPE_INFO_BRANCH:
				{
					InfoBranchNodeHistory* info_branch_node_history = (InfoBranchNodeHistory*)it->second;
					if (info_branch_node_history->is_branch) {
						input_vals[i_index] = 1.0;
					} else {
						input_vals[i_index] = -1.0;
					}
				}
				break;
			}
		}
	}

	this->network->activate(input_vals);
	double predicted_impact = this->network->output->acti_vals[0];

	double misguess = (target_impact - predicted_impact) * (target_impact - predicted_impact);
	this->misguess_histories.push_back(misguess);

	this->state_iter++;
	if (this->state_iter >= NUM_DATAPOINTS) {
		double sum_misguesses = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_misguesses += this->misguess_histories[m_index];
		}
		double new_average_misguess = sum_misguesses / NUM_DATAPOINTS;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%2 == 0) {
		#else
		double sum_misguess_variance = 0.0;
		for (int m_index = 0; m_index < NUM_DATAPOINTS; m_index++) {
			sum_misguess_variance += (this->misguess_histories[m_index] - new_average_misguess) * (this->misguess_histories[m_index] - new_average_misguess);
		}
		double new_misguess_standard_deviation = sqrt(sum_misguess_variance / NUM_DATAPOINTS);
		if (new_misguess_standard_deviation < MIN_STANDARD_DEVIATION) {
			new_misguess_standard_deviation = MIN_STANDARD_DEVIATION;
		}

		this->misguess_histories.clear();

		double misguess_improvement = solution->explore_scope_average_misguess - new_average_misguess;
		double misguess_standard_deviation = min(solution->explore_scope_misguess_standard_deviation, new_misguess_standard_deviation);
		double misguess_t_score = misguess_improvement / (misguess_standard_deviation / sqrt(NUM_DATAPOINTS));

		cout << "EvalPassThrough" << endl;
		cout << "solution->explore_scope_average_misguess: " << solution->explore_scope_average_misguess << endl;
		cout << "new_average_misguess: " << new_average_misguess << endl;
		cout << "solution->explore_scope_misguess_standard_deviation: " << solution->explore_scope_misguess_standard_deviation << endl;
		cout << "new_misguess_standard_deviation: " << new_misguess_standard_deviation << endl;
		cout << "misguess_t_score: " << misguess_t_score << endl;

		if (misguess_t_score > 1.645) {
		#endif /* MDEBUG */
			cout << "EvalPassThrough" << endl;
			cout << "this->scope_context->id: " << this->scope_context->id << endl;
			cout << "this->node_context->id: " << this->node_context->id << endl;
			cout << "this->is_branch: " << this->is_branch << endl;
			if (this->info_scope != NULL) {
				cout << "this->info_scope->id: " << this->info_scope->id << endl;
			}
			cout << "new explore path:";
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					cout << " " << this->actions[s_index]->action.move;
				} else if (this->step_types[s_index] == STEP_TYPE_SCOPE) {
					cout << " E";
				}
			}
			cout << endl;

			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: " << -1 << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}

			cout << "solution->explore_scope_average_misguess: " << solution->explore_scope_average_misguess << endl;
			cout << "new_average_misguess: " << new_average_misguess << endl;

			this->result = EXPERIMENT_RESULT_SUCCESS;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
