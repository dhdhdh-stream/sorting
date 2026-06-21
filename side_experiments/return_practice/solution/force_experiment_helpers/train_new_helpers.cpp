#include "force_experiment.h"

#include <algorithm>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "predict_wrapper.h"
#include "solution.h"
#include "solution_helpers.h"
#include "state_network.h"
#include "start_node.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

void ForceExperiment::train_new_experiment_activate(ExperimentRun* run) {
	ForceExperimentHistory* history = run->force_experiment_histories[this];

	this->original_network->activate(run->state);
	history->existing_predicted = this->original_network->output->acti_vals[0];

	history->branch_obs = run->obs_histories;
	history->branch_actions = run->action_histories;

	ForceExperimentState* new_experiment_state = new ForceExperimentState(this);
	new_experiment_state->step_index = 0;
	run->experiment_context = new_experiment_state;
}

void ForceExperiment::train_new_experiment_step(int& action,
												bool& is_next,
												ExperimentRun* run) {
	ForceExperimentState* state = (ForceExperimentState*)run->experiment_context;
	if (state->step_index >= (int)this->best_actions.size()) {
		run->node_context = this->exit_next_node;

		delete run->experiment_context;
		run->experiment_context = NULL;
	} else {
		run->action_histories.push_back(this->best_actions[state->step_index]);

		// action_helper(this->best_actions[state->step_index],
		// 			  run->state,
		// 			  run->wrapper);
		action_helper_w_history(this->best_actions[state->step_index],
								run);

		action = this->best_actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

void ForceExperiment::train_new_backprop(double target_val,
										 ExperimentRun* run,
										 ForceExperimentHistory* history,
										 Wrapper* wrapper) {
	this->new_branch_obs.push_back(history->branch_obs);
	this->new_branch_actions.push_back(history->branch_actions);
	this->new_full_obs.push_back(run->obs_histories);
	this->new_full_actions.push_back(run->action_histories);
	this->new_target_vals.push_back(target_val);

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_NUM_SAMPLES) {
		// temp
		cout << "train_new" << endl;
		cout << "new explore path:";
		for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
			cout << " " << this->best_actions[s_index];
		}
		cout << endl;

		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_branch_obs.begin(), this->new_branch_obs.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_branch_actions.begin(), this->new_branch_actions.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_full_obs.begin(), this->new_full_obs.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_full_actions.begin(), this->new_full_actions.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->new_target_vals.begin(), this->new_target_vals.end(), generator_copy);
		}

		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				start_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				action_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					branch_node->branch_experiment = NULL;
				} else {
					branch_node->original_experiment = NULL;
				}
			}
			break;
		}

		int num_verify = VERIFICATION_RATIO * (double)this->new_branch_obs.size();
		int num_train = (int)this->new_branch_obs.size() - num_verify;

		uniform_int_distribution<int> sample_distribution(0, num_train-1);

		vector<vector<double>> train_new_states(num_train);
		for (int h_index = 0; h_index < num_train; h_index++) {
			vector<double> state;
			calc_state_helper(this->new_branch_obs[h_index],
							  this->new_branch_actions[h_index],
							  wrapper,
							  state);
			train_new_states[h_index] = state;
		}

		Network* branch_network = new Network(train_new_states[0].size());
		double branch_hidden_1_average_max_update = 0.0;
		double branch_hidden_2_average_max_update = 0.0;
		double branch_hidden_3_average_max_update = 0.0;
		double branch_output_average_max_update = 0.0;
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			branch_network->activate(train_new_states[index]);
			double error = this->new_target_vals[index] - branch_network->output->acti_vals[0];
			branch_network->init_backprop(error,
										  branch_hidden_1_average_max_update,
										  branch_hidden_2_average_max_update,
										  branch_hidden_3_average_max_update,
										  branch_output_average_max_update);
		}

		double existing_sum_vals = 0.0;
		int existing_count = 0;
		for (int h_index = num_train; h_index < (int)this->existing_branch_obs.size(); h_index++) {
			vector<double> state;
			calc_state_helper(this->existing_branch_obs[h_index],
							  this->existing_branch_actions[h_index],
							  wrapper,
							  state);
			this->original_network->activate(state);
			branch_network->activate(state);
			if (branch_network->output->acti_vals[0] > this->original_network->output->acti_vals[0]) {
				existing_sum_vals += this->existing_target_vals[h_index];
				existing_count++;
			}
		}
		double existing_average = existing_sum_vals / (double)existing_count;
		double new_sum_vals = 0.0;
		int new_count = 0;
		for (int h_index = num_train; h_index < (int)this->new_branch_obs.size(); h_index++) {
			vector<double> state;
			calc_state_helper(this->new_branch_obs[h_index],
							  this->new_branch_actions[h_index],
							  wrapper,
							  state);
			this->original_network->activate(state);
			branch_network->activate(state);
			if (branch_network->output->acti_vals[0] > this->original_network->output->acti_vals[0]) {
				new_sum_vals += this->new_target_vals[h_index];
				new_count++;
			}
		}
		double new_average = new_sum_vals / (double)new_count;
		double average_ratio = (existing_count + new_count) / (2.0 * num_verify);
		double predicted_local_improvement = (new_average - existing_average) * average_ratio;

		double average_instances_per_run;
		switch (node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node_context;
				average_instances_per_run = start_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node_context;
				average_instances_per_run = action_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					average_instances_per_run = branch_node->branch_average_instances_per_run;
				} else {
					average_instances_per_run = branch_node->original_average_instances_per_run;
				}
			}
			break;
		}

		double predicted_global_improvement = average_instances_per_run * predicted_local_improvement;

		// temp
		cout << "force_experiment" << endl;
		cout << "predicted_local_improvement: " << predicted_local_improvement << endl;
		cout << "predicted_global_improvement: " << predicted_global_improvement << endl;

		bool is_success = false;
		if (existing_count > 0
				&& new_count > 0
				&& predicted_local_improvement > 0.0) {
			if (wrapper->solution->force_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->force_last_scores.begin();
						it != wrapper->solution->force_last_scores.end(); it++) {
					if (predicted_global_improvement >= *it) {
						num_better_than++;
					}
				}

				double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->force_last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->force_last_scores.size() >= NUM_LAST_TRACK) {
					wrapper->solution->force_last_scores.pop_front();
				}
				wrapper->solution->force_last_scores.push_back(predicted_global_improvement);
			} else {
				wrapper->solution->force_last_scores.push_back(predicted_global_improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%3 != 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			finalize_helper(this->node_context,
							this->is_branch,
							this->best_actions,
							this->exit_next_node,
							this->original_network,
							branch_network,
							wrapper,
							RAMP_TYPE_FORCE);
			this->original_network = NULL;
		} else {
			delete branch_network;

			train_new_state_helper(wrapper);
		}

		wrapper->experiment_iter++;

		delete this;
	}
}
