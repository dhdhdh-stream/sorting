#include "explore_experiment.h"

#include <iostream>

#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "init_network.h"
#include "noop_node.h"
#include "obs_network.h"
#include "pass_through_network.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "transition_network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_STATE_TRAIN_ITERS = 50;
const int NEW_STATE_PARTIAL_START_ITERS = 15;
#else
const int NEW_STATE_TRAIN_ITERS = 500000;
const int NEW_STATE_PARTIAL_START_ITERS = 150000;
#endif /* MDEBUG */

void ExploreExperiment::new_state_helper(SolutionWrapper* wrapper) {
	vector<InitNetwork*> potential_init_networks(this->dependencies.size());
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		Scope* scope = get_dependency_scope(this->scope_context,
											this->dependencies[d_index],
											0);
		vector<int> init_states;
		for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
			init_states.push_back(scope->num_states + s_index);
		}
		potential_init_networks[d_index] = new InitNetwork(
			init_states,
			scope->num_states + NEW_STATE_NUM_ADD,
			wrapper->solution->num_obs);
	}
	ScoreNetwork* potential_new_network = new ScoreNetwork(this->scope_context->num_states + NEW_STATE_NUM_ADD);

	uniform_int_distribution<int> new_train_distribution(0, this->new_dependencies_is_hit_histories.size()-1);
	uniform_int_distribution<int> partial_distribution(0, 19);
	for (int iter_index = 0; iter_index < NEW_STATE_TRAIN_ITERS; iter_index++) {
		int rand_index = new_train_distribution(generator);

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);

		vector<bool> is_activate(this->dependencies.size(), false);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (this->new_dependencies_is_hit_histories[rand_index][d_index]) {
				if (iter_index < NEW_STATE_PARTIAL_START_ITERS
						|| partial_distribution(generator) != 0) {
					is_activate[d_index] = true;

					potential_init_networks[d_index]->init_activate(
						this->new_dependencies_state_histories[rand_index][d_index],
						new_state,
						this->new_dependencies_obs_histories[rand_index][d_index]);
				}
			}
		}

		potential_new_network->init_activate(this->new_state_histories[rand_index],
											 new_state);

		vector<double> new_state_errors(NEW_STATE_NUM_ADD, 0.0);

		if (this->use_signal) {
			potential_new_network->init_backprop(this->new_signal_histories[rand_index],
												 new_state_errors);
		} else {
			potential_new_network->init_backprop(this->new_target_val_histories[rand_index],
												 new_state_errors);
		}

		for (int d_index = (int)this->dependencies.size()-1; d_index >= 0; d_index--) {
			if (is_activate[d_index]) {
				potential_init_networks[d_index]->init_backprop(new_state_errors);
			}
		}

		if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				potential_init_networks[d_index]->init_update();
			}
			potential_new_network->init_update();
		}
	}
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		for (int i_index = 0; i_index < (int)potential_init_networks[d_index]->state_input->errors.size(); i_index++) {
			potential_init_networks[d_index]->state_input->errors(i_index) = 0.0;
		}
	}
	for (int i_index = 0; i_index < (int)potential_new_network->state_input->errors.size(); i_index++) {
		potential_new_network->state_input->errors(i_index) = 0.0;
	}

	double existing_sum_vals = 0.0;
	int existing_count = 0;
	for (int h_index = 0; h_index < (int)this->existing_dependencies_is_hit_histories.size(); h_index++) {
		this->existing_network->activate(this->existing_state_histories[h_index]);
		double existing_predicted = this->existing_network->output->acti_vals[0];

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (this->existing_dependencies_is_hit_histories[h_index][d_index]) {
				potential_init_networks[d_index]->init_activate(
					this->existing_dependencies_state_histories[h_index][d_index],
					new_state,
					this->existing_dependencies_obs_histories[h_index][d_index]);
			}
		}
		potential_new_network->init_activate(this->existing_state_histories[h_index],
											 new_state);
		double new_predicted = potential_new_network->output->acti_vals[0];

		if (new_predicted >= existing_predicted) {
			existing_sum_vals += this->existing_target_val_histories[h_index];
			existing_count++;
		}
	}
	double existing_average = existing_sum_vals / (double)existing_count;
	double new_sum_vals = 0.0;
	int new_count = 0;
	for (int h_index = 0; h_index < (int)this->new_dependencies_is_hit_histories.size(); h_index++) {
		this->existing_network->activate(this->new_state_histories[h_index]);
		double existing_predicted = this->existing_network->output->acti_vals[0];

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (this->new_dependencies_is_hit_histories[h_index][d_index]) {
				potential_init_networks[d_index]->init_activate(
					this->new_dependencies_state_histories[h_index][d_index],
					new_state,
					this->new_dependencies_obs_histories[h_index][d_index]);
			}
		}
		potential_new_network->init_activate(this->new_state_histories[h_index],
											 new_state);
		double new_predicted = potential_new_network->output->acti_vals[0];

		if (new_predicted >= existing_predicted) {
			new_sum_vals += this->new_target_val_histories[h_index];
			new_count++;
		}
	}
	double new_average = new_sum_vals / (double)new_count;
	double average_ratio = (existing_count + new_count)
		/ ((double)this->existing_dependencies_is_hit_histories.size()
			+ (double)this->new_dependencies_is_hit_histories.size());
	double local_improvement = (new_average - existing_average) * average_ratio;

	double average_instances_per_run;
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			average_instances_per_run = noop_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			average_instances_per_run = action_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			average_instances_per_run = scope_node->average_instances_per_run;
		}
		break;
	default:
	// case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				average_instances_per_run = branch_node->branch_average_instances_per_run;
			} else {
				average_instances_per_run = branch_node->original_average_instances_per_run;
			}
		}
		break;
	}
	double global_improvement = average_instances_per_run * local_improvement;

	// // temp
	// cout << "new_state" << endl;
	// cout << "this->scope_context->id: " << this->scope_context->id << endl;
	// cout << "local_improvement: " << local_improvement << endl;
	// cout << "global_improvement: " << global_improvement << endl;

	bool is_success = false;
	if (local_improvement > 0.0) {
		if (this->scope_context->train_new_state_last_scores.size() >= MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = this->scope_context->train_new_state_last_scores.begin();
					it != this->scope_context->train_new_state_last_scores.end(); it++) {
				if (global_improvement >= *it) {
					num_better_than++;
				}
			}

			double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->train_new_state_last_scores.size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			}

			if (this->scope_context->train_new_state_last_scores.size() >= NUM_LAST_TRACK) {
				this->scope_context->train_new_state_last_scores.pop_front();
			}
			this->scope_context->train_new_state_last_scores.push_back(global_improvement);
		} else {
			this->scope_context->train_new_state_last_scores.push_back(global_improvement);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (is_success || rand()%3 != 0) {
	#else
	if (is_success) {
	#endif /* MDEBUG */
		this->new_network = potential_new_network;
		this->init_networks = potential_init_networks;

		this->sum_vals = 0.0;

		this->state = EXPLORE_EXPERIMENT_STATE_NEW_STATE_MEASURE;
		this->state_iter = 0;
	} else {
		for (int n_index = 0; n_index < (int)potential_init_networks.size(); n_index++) {
			delete potential_init_networks[n_index];
		}
		delete potential_new_network;

		delete this;
	}
}
