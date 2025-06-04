#include "pass_through_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	if (this->is_branch == is_branch) {
		run_helper.num_experiment_instances++;

		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
			= run_helper.experiment_histories.find(this);
		PassThroughExperimentHistory* history;
		if (it == run_helper.experiment_histories.end()) {
			history = new PassThroughExperimentHistory(this);
			run_helper.experiment_histories[this] = history;
		} else {
			history = (PassThroughExperimentHistory*)it->second;
		}

		if (history->is_active) {
			for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
				if (this->step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->actions[s_index]);

					run_helper.num_actions++;
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[s_index]);
					this->scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}
			}

			curr_node = this->exit_next_node;
		}
	}
}

void PassThroughExperiment::backprop(double target_val,
									 RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories[this];

	if (history->is_active) {
		this->new_sum_score += target_val;
		this->new_count++;
	} else {
		this->existing_sum_score += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_NUM_DATAPOINTS) {
		double existing_score = this->existing_sum_score / this->existing_count;
		double new_score = this->new_sum_score / this->new_count;

		#if defined(MDEBUG) && MDEBUG
		if (rand()%4 != 0) {
		#else
		if (new_score > existing_score) {
		#endif /* MDEBUG */
			this->existing_sum_score = 0.0;
			this->existing_count = 0;
			this->new_sum_score = 0.0;
			this->new_count = 0;

			this->state_iter = 0;

			switch (this->state) {
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT;
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT;
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT;
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT;
				break;
			case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
				this->improvement = new_score - existing_score;

				cout << "PassThrough" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->actions[s_index].move;
					} else {
						cout << " E" << this->scopes[s_index]->id;
					}
				}
				cout << endl;

				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;

				break;
			}
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
