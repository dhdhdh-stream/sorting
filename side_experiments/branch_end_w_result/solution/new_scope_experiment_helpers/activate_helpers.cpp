#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_wrapper.h"
#include "start_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_C1_NUM_SAMPLES = 1;
const int INITIAL_C2_NUM_SAMPLES = 2;
const int INITIAL_C3_NUM_SAMPLES = 3;
const int INITIAL_C4_NUM_SAMPLES = 4;
#else
const int INITIAL_C1_NUM_SAMPLES = 1;
const int INITIAL_C2_NUM_SAMPLES = 10;
const int INITIAL_C3_NUM_SAMPLES = 100;
const int INITIAL_C4_NUM_SAMPLES = 1000;
#endif /* MDEBUG */

void NewScopeExperiment::result_check_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		if (wrapper->curr_new_scope_experiment->curr_experiment == this) {
			NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;
			history->is_hit = true;
		} else {
			NewScopeExperimentState* new_experiment_state = new NewScopeExperimentState(this);
			wrapper->experiment_context.back() = new_experiment_state;

			ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(this->new_scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);
		}
	}
}

void NewScopeExperiment::check_activate(AbstractNode* experiment_node,
										bool is_branch,
										SolutionWrapper* wrapper) {
	if (is_branch == this->is_branch) {
		NewScopeExperimentState* new_experiment_state = new NewScopeExperimentState(this);
		wrapper->experiment_context.back() = new_experiment_state;

		ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
		wrapper->scope_histories.push_back(inner_scope_history);
		wrapper->node_context.push_back(this->new_scope->nodes[0]);
		wrapper->experiment_context.push_back(NULL);
	}
}

void NewScopeExperiment::experiment_step(vector<double>& obs,
										 int& action,
										 bool& is_next,
										 bool& fetch_action,
										 SolutionWrapper* wrapper) {
	NewScopeExperimentState* experiment_state = (NewScopeExperimentState*)wrapper->experiment_context.back();

	wrapper->node_context.back() = this->exit_next_node;

	delete experiment_state;
	wrapper->experiment_context.back() = NULL;
}

void NewScopeExperiment::set_action(int action,
									SolutionWrapper* wrapper) {
	// do nothing
}

void NewScopeExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();
}

void NewScopeExperiment::backprop(double target_val,
								  SolutionWrapper* wrapper) {
	if (this->new_scope_histories.size() < MEASURE_ITERS) {
		this->new_scope_histories.push_back(wrapper->scope_histories[0]);
		this->new_target_val_histories.push_back(target_val);
	} else {
		uniform_int_distribution<int> distribution(0, this->new_scope_histories.size()-1);
		int random_index = distribution(generator);
		delete this->new_scope_histories[random_index];
		this->new_scope_histories[random_index] = wrapper->scope_histories[0];
		this->new_target_val_histories[random_index] = target_val;
	}

	NewScopeExperimentHistory* history = (NewScopeExperimentHistory*)wrapper->experiment_history;
	if (history->is_hit) {
		this->new_sum_scores += target_val - wrapper->existing_result;

		this->state_iter++;
		bool is_eval = false;
		switch (this->state) {
		case NEW_SCOPE_EXPERIMENT_STATE_C1:
			if (this->state_iter >= INITIAL_C1_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case NEW_SCOPE_EXPERIMENT_STATE_C2:
			if (this->state_iter >= INITIAL_C2_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case NEW_SCOPE_EXPERIMENT_STATE_C3:
			if (this->state_iter >= INITIAL_C3_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		case NEW_SCOPE_EXPERIMENT_STATE_C4:
			if (this->state_iter >= INITIAL_C4_NUM_SAMPLES) {
				is_eval = true;
			}
			break;
		}

		if (is_eval) {
			double new_score_average = this->new_sum_scores / (double)this->state_iter;

			#if defined(MDEBUG) && MDEBUG
			if (new_score_average > 0.0 || rand()%5 != 0) {
			#else
			if (new_score_average > 0.0) {
			#endif /* MDEBUG */
				switch (this->state) {
				case NEW_SCOPE_EXPERIMENT_STATE_C1:
					this->state = NEW_SCOPE_EXPERIMENT_STATE_C2;
					break;
				case NEW_SCOPE_EXPERIMENT_STATE_C2:
					this->state = NEW_SCOPE_EXPERIMENT_STATE_C3;
					break;
				case NEW_SCOPE_EXPERIMENT_STATE_C3:
					this->state = NEW_SCOPE_EXPERIMENT_STATE_C4;
					break;
				case NEW_SCOPE_EXPERIMENT_STATE_C4:
					double average_hits_per_run;
					switch (this->node_context->type) {
					case NODE_TYPE_START:
						{
							StartNode* start_node = (StartNode*)this->node_context;
							average_hits_per_run = start_node->average_hits_per_run;
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							average_hits_per_run = action_node->average_hits_per_run;
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;
							average_hits_per_run = scope_node->average_hits_per_run;
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								average_hits_per_run = branch_node->branch_average_hits_per_run;
							} else {
								average_hits_per_run = branch_node->original_average_hits_per_run;
							}
						}
						break;
					case NODE_TYPE_OBS:
						{
							ObsNode* obs_node = (ObsNode*)this->node_context;
							average_hits_per_run = obs_node->average_hits_per_run;
						}
						break;
					}

					this->improvement = average_hits_per_run * new_score_average;

					// cout << "NewScopeExperiment" << endl;
					// cout << "this->scope_context->id: " << this->scope_context->id << endl;
					// cout << "this->node_context->id: " << this->node_context->id << endl;
					// cout << "this->is_branch: " << this->is_branch << endl;

					// double improvement = this->new_score - existing_score;
					// cout << "improvement: " << improvement << endl;

					// cout << endl;

					this->result = EXPERIMENT_RESULT_SUCCESS;
					break;
				}
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}
	}
}
