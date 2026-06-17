#include "wrapper.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "force_experiment.h"
#include "globals.h"
#include "predict_wrapper.h"
#include "solution.h"
#include "solution_helpers.h"
#include "start_node.h"
#include "utilities.h"
#include "world_model.h"
#include "world_model_helpers.h"

using namespace std;

const int TARGET_NODES_PER_EVAL = 20;

const int EXPERIMENT_REFRESH_NUM_ITERS = 20;

void Wrapper::experiment_init(ExperimentRun* run) {
	run->wrapper = this;

	run->node_context = this->solution->nodes[0];
	run->experiment_context = NULL;

	run->state = vector<double>(this->world_model->num_states, 0.0);

	this->iter++;

	uniform_int_distribution<int> explore_distribution(0, 1);
	if (explore_distribution(generator) == 0) {
		run->should_force = true;
	} else {
		run->should_force = false;
	}

	#if defined(MDEBUG) && MDEBUG
	this->run_index++;
	this->starting_run_seed = this->run_index;
	this->curr_run_seed = xorshift(this->starting_run_seed);
	#endif /* MDEBUG */
}

pair<bool,int> Wrapper::experiment_step(vector<double> obs,
										ExperimentRun* run) {
	run->obs_histories.push_back(obs);

	obs_helper(obs,
			   run->state,
			   this);

	if (run->experiment_context == NULL
			&& run->node_context != NULL) {
		run->node_context->experiment_step_start(run);
	}

	int action;
	bool is_next = false;
	bool is_done = false;
	while (!is_next) {
		if (run->node_context == NULL
				&& run->experiment_context == NULL) {
			is_next = true;
			is_done = true;
		} else if (run->experiment_context != NULL) {
			run->experiment_context->experiment->experiment_step(
				action,
				is_next,
				run);
		} else {
			run->node_context->experiment_step(action,
											   is_next,
											   run);
		}
	}

	return pair<bool,int>{is_done, action};
}

void Wrapper::experiment_end(double result,
							 ExperimentRun* run) {
	if (run->should_force) {
		if (run->force_experiment_histories.size() == 0) {
			if (this->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
				for (map<int, AbstractNode*>::iterator it = this->solution->nodes.begin();
						it != this->solution->nodes.end(); it++) {
					switch (it->second->type) {
					case NODE_TYPE_START:
						{
							StartNode* start_node = (StartNode*)it->second;
							if (start_node->experiment != NULL) {
								delete start_node->experiment;
								start_node->experiment = NULL;
							}
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->second;
							if (action_node->experiment != NULL) {
								delete action_node->experiment;
								action_node->experiment = NULL;
							}
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)it->second;
							if (branch_node->original_experiment != NULL) {
								delete branch_node->original_experiment;
								branch_node->original_experiment = NULL;
							}
							if (branch_node->branch_experiment != NULL) {
								delete branch_node->branch_experiment;
								branch_node->branch_experiment = NULL;
							}
						}
						break;
					}
				}

				this->experiment_iter = 0;
			}

			create_force_experiment(run,
									this);
		} else if (run->force_experiment_histories.size() >= 2) {
			ForceExperiment* keep_experiment = NULL;
			for (map<ForceExperiment*, ForceExperimentHistory*>::iterator it = run->force_experiment_histories.begin();
					it != run->force_experiment_histories.end(); it++) {
				if (keep_experiment == NULL) {
					keep_experiment = it->first;
				} else {
					if (it->first->further_than(keep_experiment)) {
						switch (keep_experiment->node_context->type) {
							case NODE_TYPE_START:
							{
								StartNode* start_node = (StartNode*)keep_experiment->node_context;
								start_node->experiment = NULL;
							}
							break;
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)keep_experiment->node_context;
								action_node->experiment = NULL;
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)keep_experiment->node_context;
								if (keep_experiment->is_branch) {
									branch_node->branch_experiment = NULL;
								} else {
									branch_node->original_experiment = NULL;
								}
							}
							break;
						}
						delete keep_experiment;

						keep_experiment = it->first;
					} else {
						switch (it->first->node_context->type) {
							case NODE_TYPE_START:
							{
								StartNode* start_node = (StartNode*)it->first->node_context;
								start_node->experiment = NULL;
							}
							break;
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)it->first->node_context;
								action_node->experiment = NULL;
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)it->first->node_context;
								if (it->first->is_branch) {
									branch_node->branch_experiment = NULL;
								} else {
									branch_node->original_experiment = NULL;
								}
							}
							break;
						}
						delete it->first;
					}
				}
			}
		} else {
			for (map<ForceExperiment*, ForceExperimentHistory*>::iterator it = run->force_experiment_histories.begin();
					it != run->force_experiment_histories.end(); it++) {
				it->first->backprop(result,
									run,
									it->second,
									this);
			}
		}
	} else {
		if (this->solution->score_histories.size() < SCORE_HISTORIES_NUM_SAVE) {
			this->solution->score_histories.push_back(result);
		} else {
			this->solution->score_histories[this->solution->score_index] = result;
			this->solution->score_index++;
			if (this->solution->score_index >= SCORE_HISTORIES_NUM_SAVE) {
				this->solution->score_index = 0;
			}
		}

		// int node_count = 0;
		// int eval_count = 0;
		// count_eval_helper(run,
		// 				  node_count,
		// 				  eval_count);

		// int target_count = (node_count + (TARGET_NODES_PER_EVAL-1)) / TARGET_NODES_PER_EVAL;
		// if (eval_count < target_count) {
		// 	// temp
		// 	cout << "node_count: " << node_count << endl;
		// 	cout << "eval_count: " << eval_count << endl;
		// 	cout << "target_count: " << target_count << endl;
		// 	create_experiment(run,
		// 					  this);
		// }

		update_solution_helper(run,
							   result,
							   this);

		// // temp
		// if ((this->iter + 1) % 1000 == 0) {
		// 	cout << this->iter << endl;
		// 	double score_average;
		// 	double misguess_average;
		// 	measure_helper(this,
		// 				   score_average,
		// 				   misguess_average);
		// 	cout << "score_average: " << score_average << endl;
		// 	cout << "misguess_average: " << misguess_average << endl;
		// 	cout << "this->world_model->predict->misguess_average: " << this->world_model->predict->misguess_average << endl;
		// 	cout << "this->world_model->candidate_predict->misguess_average: " << this->world_model->candidate_predict->misguess_average << endl;
		// 	cout << "this->world_model->num_states: " << this->world_model->num_states << endl;
		// 	cout << endl;
		// }
	}

	// if (this->sample_obs.size() < SAMPLES_NUM_SAVE) {
	// 	this->sample_obs.push_back(run->obs_histories);
	// 	this->sample_actions.push_back(run->action_histories);
	// 	this->sample_target_vals.push_back(result);
	// } else {
	// 	this->sample_obs[this->sample_index] = run->obs_histories;
	// 	this->sample_actions[this->sample_index] = run->action_histories;
	// 	this->sample_target_vals[this->sample_index] = result;
	// }
	// this->sample_index++;
	// if (this->sample_index >= SAMPLES_NUM_SAVE) {
	// 	this->sample_index = 0;
	// }

	// update_world_model_helper(this);
}
