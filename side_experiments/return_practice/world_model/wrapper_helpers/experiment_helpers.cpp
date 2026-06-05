#include "wrapper.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "crazy.h"
#include "experiment.h"
#include "experiment_run.h"
#include "globals.h"
#include "solution.h"
#include "solution_helpers.h"
#include "utilities.h"
#include "world_model.h"
#include "world_model_helpers.h"

using namespace std;

const int TARGET_NODES_PER_EVAL = 10;

void Wrapper::experiment_init(ExperimentRun* run) {
	run->wrapper = this;

	run->node_context = this->solution->nodes[0];
	run->experiment_context = NULL;

	run->state = vector<double>(this->curr_model->num_states, 0.0);

	this->iter++;

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
	if (this->crazy != NULL) {
		delete this->crazy;
		this->crazy = NULL;

		// if (this->hit_crazy) {
		// 	cout << "crazy actions:";
		// 	for (int a_index = 0; a_index < (int)run->action_histories.size(); a_index++) {
		// 		cout << " " << run->action_histories[a_index];
		// 	}
		// 	cout << endl;
		// }

		if (this->sample_obs.size() < SAMPLES_NUM_SAVE) {
			this->sample_obs.push_back(run->obs_histories);
			this->sample_actions.push_back(run->action_histories);
			this->sample_target_vals.push_back(result);
		} else {
			this->sample_obs[this->sample_index] = run->obs_histories;
			this->sample_actions[this->sample_index] = run->action_histories;
			this->sample_target_vals[this->sample_index] = result;
		}
		this->sample_index++;
		if (this->sample_index >= SAMPLES_NUM_SAVE) {
			check_state_size_helper(this);

			this->sample_index = 0;
		}

		update_world_model_helper(this);
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
	}

	update_solution_helper(run,
						   result);

	// temp
	if (this->iter % 2000 == 0) {
		double score_average;
		double misguess_average;
		measure_helper(this,
					   score_average,
					   misguess_average);
		cout << endl;
		cout << "score_average: " << score_average << endl;
		cout << "misguess_average: " << misguess_average << endl;
		cout << "this->curr_model->num_states: " << this->curr_model->num_states << endl;
		measure_test(this);
		cout << endl;
	}

	int node_count = 0;
	int eval_count = 0;
	count_eval_helper(run,
					  node_count,
					  eval_count);

	int target_count = (node_count + (TARGET_NODES_PER_EVAL-1)) / TARGET_NODES_PER_EVAL;
	if (eval_count < target_count) {
		create_experiment(run,
						  this);
	}

	for (map<Experiment*, ExperimentHistory*>::iterator it = run->experiment_histories.begin();
			it != run->experiment_histories.end(); it++) {
		it->first->backprop(result,
							it->second,
							this);
	}

	uniform_int_distribution<int> crazy_distribution(0, 4);
	if (crazy_distribution(generator) == 0) {
		create_crazy(run,
					 this);
	}
}
