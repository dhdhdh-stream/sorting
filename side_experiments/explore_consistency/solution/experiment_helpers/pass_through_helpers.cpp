#include "experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "scope.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int C1_NUM_SAMPLES = 1;
const int C2_NUM_SAMPLES = 2;
const int C3_NUM_SAMPLES = 3;
#else
const int C1_NUM_SAMPLES = 10;
const int C2_NUM_SAMPLES = 100;
const int C3_NUM_SAMPLES = 1000;
#endif /* MDEBUG */

void Experiment::pass_through_result_backprop(double target_val,
											  SolutionWrapper* wrapper) {
	ExperimentHistory* history = (ExperimentHistory*)wrapper->experiment_history;
	if (!history->is_hit) {
		this->total_count++;
		this->total_sum_scores += target_val;
	}
}

void Experiment::pass_through_check_activate(SolutionWrapper* wrapper) {
	ExperimentState* new_experiment_state = new ExperimentState(this);
	new_experiment_state->step_index = 0;
	wrapper->experiment_context.back() = new_experiment_state;
}

void Experiment::pass_through_step(vector<double>& obs,
								   int& action,
								   bool& is_next,
								   SolutionWrapper* wrapper) {
	ExperimentState* experiment_state = (ExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->actions[experiment_state->step_index];
			is_next = true;

			wrapper->num_actions++;

			experiment_state->step_index++;
		} else {
			ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[experiment_state->step_index]);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->scopes[experiment_state->step_index]->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void Experiment::pass_through_exit_step(SolutionWrapper* wrapper,
										ExperimentState* experiment_state) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void Experiment::pass_through_backprop(double target_val,
									   SolutionWrapper* wrapper) {
	this->total_count++;
	this->total_sum_scores += target_val;

	this->sum_scores += (target_val - wrapper->existing_result);

	this->state_iter++;
	bool is_eval = false;
	switch (this->state) {
	case EXPERIMENT_STATE_PASS_THROUGH_C1:
		if (this->state_iter >= C1_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case EXPERIMENT_STATE_PASS_THROUGH_C2:
		if (this->state_iter >= C2_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	case EXPERIMENT_STATE_PASS_THROUGH_C3:
		if (this->state_iter >= C3_NUM_SAMPLES) {
			is_eval = true;
		}
		break;
	}
	if (is_eval) {
		double new_score = this->sum_scores / this->state_iter;

		#if defined(MDEBUG) && MDEBUG
		if (new_score > 0.0 || rand()%5 != 0) {
		#else
		if (new_score > 0.0) {
		#endif /* MDEBUG */
			switch (this->state) {
			case EXPERIMENT_STATE_PASS_THROUGH_C1:
				this->state = EXPERIMENT_STATE_PASS_THROUGH_C2;
				break;
			case EXPERIMENT_STATE_PASS_THROUGH_C2:
				this->state = EXPERIMENT_STATE_PASS_THROUGH_C3;
				break;
			case EXPERIMENT_STATE_PASS_THROUGH_C3:
				double average_hits_per_run = (double)C3_NUM_SAMPLES / (double)this->total_count;

				this->improvement = average_hits_per_run * new_score;

				this->is_pass_through = true;

				cout << "pass_through" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
					if (this->step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->actions[s_index];
					} else {
						cout << " E" << this->scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->exit_next_node == NULL) {
					cout << "this->exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
				}

				cout << "average_hits_per_run: " << average_hits_per_run << endl;
				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				this->result = EXPERIMENT_RESULT_SUCCESS;
				break;
			}
		} else {
			this->is_pass_through = false;

			this->state = EXPERIMENT_STATE_TRAIN_NEW;
			this->state_iter = 0;
		}
	}
}
