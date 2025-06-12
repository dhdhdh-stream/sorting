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
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 2;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 2;
#else
const int MEASURE_1_PERCENT_NUM_DATAPOINTS = 10;
const int MEASURE_5_PERCENT_NUM_DATAPOINTS = 50;
const int MEASURE_10_PERCENT_NUM_DATAPOINTS = 100;
const int MEASURE_25_PERCENT_NUM_DATAPOINTS = 250;
const int MEASURE_50_PERCENT_NUM_DATAPOINTS = 500;
#endif /* MDEBUG */

void PassThroughExperiment::activate(AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
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

void PassThroughExperiment::backprop(double target_val,
									 AbstractExperimentHistory* history,
									 set<Scope*>& updated_scopes) {
	PassThroughExperimentHistory* pass_through_experiment_history = (PassThroughExperimentHistory*)history;

	if (pass_through_experiment_history->is_active) {
		this->new_sum_score += target_val;
		this->new_count++;
	} else {
		this->existing_sum_score += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	switch (this->state) {
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_1_PERCENT:
		if (this->new_count >= MEASURE_1_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_5_PERCENT:
		if (this->new_count >= MEASURE_5_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_10_PERCENT:
		if (this->new_count >= MEASURE_10_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_25_PERCENT:
		if (this->new_count >= MEASURE_25_PERCENT_NUM_DATAPOINTS) {
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			if (new_score > existing_score) {
			#endif /* MDEBUG */
				this->existing_sum_score = 0.0;
				this->existing_count = 0;
				this->new_sum_score = 0.0;
				this->new_count = 0;

				this->state = PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT;
				this->state_iter = 0;
			} else {
				delete this;
				return;
			}
		}
		break;
	case PASS_THROUGH_EXPERIMENT_STATE_MEASURE_50_PERCENT:
		if (this->new_count >= MEASURE_50_PERCENT_NUM_DATAPOINTS) {
			double existing_score = this->existing_sum_score / this->existing_count;
			double new_score = this->new_sum_score / this->new_count;
			#if defined(MDEBUG) && MDEBUG
			if (rand()%4 != 0) {
			#else
			if (new_score > existing_score) {
			#endif /* MDEBUG */
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

				double improvement = new_score - existing_score;
				cout << "improvement: " << improvement << endl;

				cout << endl;

				add();
				updated_scopes.insert(this->scope_context);
				delete this;
			} else {
				this->scope_context->generalize_iter++;

				delete this;
				return;
			}
		}
		break;
	}
}
