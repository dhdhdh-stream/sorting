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
	int num_existing_train = (1.0 - VERIFY_RATIO) * (double)this->existing_dependencies_is_hit_histories.size();

	vector<InitNetwork*> init_networks(this->dependencies.size());
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		Scope* scope = get_dependency_scope(this->scope_context,
											this->dependencies[d_index],
											0);
		vector<int> init_states;
		for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
			init_states.push_back(scope->num_states + s_index);
		}
		init_networks[d_index] = new InitNetwork(init_states,
												 scope->num_states + NEW_STATE_NUM_ADD,
												 wrapper->solution->num_obs);
	}
	ScoreNetwork* new_network = new ScoreNetwork(this->scope_context->num_states + NEW_STATE_NUM_ADD);

	int num_new_train = (1.0 - VERIFY_RATIO) * (double)this->new_dependencies_is_hit_histories.size();

	uniform_int_distribution<int> new_train_distribution(0, num_new_train-1);
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

					init_networks[d_index]->init_activate(this->new_dependencies_state_histories[rand_index][d_index],
														  new_state,
														  this->new_dependencies_obs_histories[rand_index][d_index]);
				}
			}
		}

		new_network->init_activate(this->new_state_histories[rand_index],
								   new_state);

		vector<double> new_state_errors(NEW_STATE_NUM_ADD, 0.0);

		if (this->use_signal) {
			new_network->init_backprop(this->new_signal_histories[rand_index],
									   new_state_errors);
		} else {
			new_network->init_backprop(this->new_target_val_histories[rand_index],
									   new_state_errors);
		}

		for (int d_index = (int)this->dependencies.size()-1; d_index >= 0; d_index--) {
			if (is_activate[d_index]) {
				init_networks[d_index]->init_backprop(new_state_errors);
			}
		}

		if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
			for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
				init_networks[d_index]->init_update();
			}
			new_network->init_update();
		}
	}
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		for (int i_index = 0; i_index < (int)init_networks[d_index]->state_input->errors.size(); i_index++) {
			init_networks[d_index]->state_input->errors(i_index) = 0.0;
		}
	}
	for (int i_index = 0; i_index < (int)new_network->state_input->errors.size(); i_index++) {
		new_network->state_input->errors(i_index) = 0.0;
	}

	double existing_sum_vals = 0.0;
	int existing_count = 0;
	for (int h_index = num_existing_train; h_index < (int)this->existing_dependencies_is_hit_histories.size(); h_index++) {
		this->existing_network->activate(this->existing_state_histories[h_index]);

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (this->existing_dependencies_is_hit_histories[h_index][d_index]) {
				init_networks[d_index]->init_activate(this->existing_dependencies_state_histories[h_index][d_index],
													  new_state,
													  this->existing_dependencies_obs_histories[h_index][d_index]);
			}
		}
		new_network->init_activate(this->existing_state_histories[h_index],
								   new_state);

		if (new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			existing_sum_vals += this->existing_target_val_histories[h_index];
			existing_count++;
		}
	}
	double existing_average = existing_sum_vals / (double)existing_count;
	double new_sum_vals = 0.0;
	int new_count = 0;
	for (int h_index = num_new_train; h_index < (int)this->new_dependencies_is_hit_histories.size(); h_index++) {
		this->existing_network->activate(this->new_state_histories[h_index]);

		vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			if (this->new_dependencies_is_hit_histories[h_index][d_index]) {
				init_networks[d_index]->init_activate(this->new_dependencies_state_histories[h_index][d_index],
													  new_state,
													  this->new_dependencies_obs_histories[h_index][d_index]);
			}
		}
		new_network->init_activate(this->new_state_histories[h_index],
								   new_state);

		if (new_network->output->acti_vals(0) >= this->existing_network->output->acti_vals(0)) {
			new_sum_vals += this->new_target_val_histories[h_index];
			new_count++;
		}
	}
	double new_average = new_sum_vals / (double)new_count;
	double average_ratio = (existing_count + new_count)
		/ ((double)this->existing_dependencies_is_hit_histories.size() - num_existing_train
			+ (double)this->new_dependencies_is_hit_histories.size() - num_new_train);
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
		if (this->scope_context->new_state_last_scores.size() >= NEW_STATE_MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = this->scope_context->new_state_last_scores.begin();
					it != this->scope_context->new_state_last_scores.end(); it++) {
				if (global_improvement >= *it) {
					num_better_than++;
				}
			}

			double target_better_than = NEW_STATE_LAST_BETTER_THAN_RATIO * (double)this->scope_context->new_state_last_scores.size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			}

			if (this->scope_context->new_state_last_scores.size() >= NEW_STATE_NUM_LAST_TRACK) {
				this->scope_context->new_state_last_scores.pop_front();
			}
			this->scope_context->new_state_last_scores.push_back(global_improvement);
		} else {
			this->scope_context->new_state_last_scores.push_back(global_improvement);
		}
	}

	#if defined(MDEBUG) && MDEBUG
	if (is_success || rand()%3 != 0) {
	#else
	if (is_success) {
	#endif /* MDEBUG */
		// // temp
		// cout << "this->dependencies:" << endl;
		// for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		// 	cout << d_index << ":";
		// 	for (int l_index = 0; l_index < (int)this->dependencies[d_index].size(); l_index++) {
		// 		cout << " " << this->dependencies[d_index][l_index];
		// 	}
		// 	cout << endl;
		// }

		// set<Scope*> scopes_needed;
		// set<ScopeNode*> transitions_needed;
		// scopes_needed.insert(this->scope_context);
		// for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		// 	get_dependency_changes_helper(this->scope_context,
		// 								  this->dependencies[d_index],
		// 								  0,
		// 								  scopes_needed,
		// 								  transitions_needed);
		// }

		// for (set<ScopeNode*>::iterator it = transitions_needed.begin();
		// 		it != transitions_needed.end(); it++) {
		// 	ScopeNode* scope_node = *it;
		// 	int out_num_states = scope_node->parent->num_states;
		// 	int in_num_states = scope_node->scope->num_states;
		// 	for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
		// 		PassThroughNetwork* new_in_pass_through_network = new PassThroughNetwork(
		// 			out_num_states + s_index,
		// 			in_num_states + s_index);
		// 		scope_node->in_pass_through_networks.push_back(new_in_pass_through_network);

		// 		PassThroughNetwork* new_out_pass_through_network = new PassThroughNetwork(
		// 			in_num_states + s_index,
		// 			out_num_states + s_index);
		// 		scope_node->out_pass_through_networks.push_back(new_out_pass_through_network);
		// 	}
		// }

		// for (set<Scope*>::iterator it = scopes_needed.begin();
		// 		it != scopes_needed.end(); it++) {
		// 	Scope* scope = *it;

		// 	scope->num_states += NEW_STATE_NUM_ADD;

		// 	scope->start_obs_network->add_states(scope->num_states);
		// 	for (int n_index = 0; n_index < (int)scope->start_init_networks.size(); n_index++) {
		// 		scope->start_init_networks[n_index]->add_states(scope->num_states);
		// 	}
		// 	scope->end_score_network->add_states(scope->num_states);

		// 	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		// 			it != scope->nodes.end(); it++) {
		// 		switch (it->second->type) {
		// 		case NODE_TYPE_ACTION:
		// 			{
		// 				ActionNode* action_node = (ActionNode*)it->second;

		// 				action_node->action_network->add_states(scope->num_states);
		// 				action_node->obs_network->add_states(scope->num_states);
		// 				for (int n_index = 0; n_index < (int)action_node->init_networks.size(); n_index++) {
		// 					action_node->init_networks[n_index]->add_states(scope->num_states);
		// 				}
		// 			}
		// 			break;
		// 		case NODE_TYPE_SCOPE:
		// 			{
		// 				ScopeNode* scope_node = (ScopeNode*)it->second;

		// 				scope_node->in_network->add_front_states(scope->num_states);
		// 				scope_node->out_network->add_back_states(scope->num_states);
		// 			}
		// 			break;
		// 		case NODE_TYPE_BRANCH:
		// 			{
		// 				BranchNode* branch_node = (BranchNode*)it->second;

		// 				branch_node->original_network->add_states(scope->num_states);
		// 				branch_node->branch_network->add_states(scope->num_states);
		// 			}
		// 			break;
		// 		}
		// 	}

		// 	for (int a_index = 0; a_index < (int)scope->generic_action_nodes.size(); a_index++) {
		// 		ActionNode* action_node = scope->generic_action_nodes[a_index];

		// 		action_node->action_network->add_states(scope->num_states);
		// 		action_node->obs_network->add_states(scope->num_states);
		// 	}

		// 	for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		// 		Scope* p_outer_scope = wrapper->solution->scopes[s_index];
		// 		for (map<int, AbstractNode*>::iterator it = p_outer_scope->nodes.begin();
		// 				it != p_outer_scope->nodes.end(); it++) {
		// 			if (it->second->type == NODE_TYPE_SCOPE) {
		// 				ScopeNode* scope_node = (ScopeNode*)it->second;
		// 				if (scope_node->scope == scope) {
		// 					scope_node->in_network->add_back_states(scope->num_states);
		// 					scope_node->out_network->add_front_states(scope->num_states);
		// 				}
		// 			}
		// 		}
		// 	}
		// }

		// for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		// 	vector<Scope*> init_network_scope_context;
		// 	add_dependency_helper(this->scope_context,
		// 						  init_network_scope_context,
		// 						  this->dependencies[d_index],
		// 						  0,
		// 						  init_networks[d_index]);
		// }

		// this->existing_network->add_states(this->scope_context->num_states);

		// add(true,
		// 	new_network,
		// 	wrapper);

		// /**
		//  * - includes "delete this"
		//  */
		// for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
		// 	Scope* scope = wrapper->solution->scopes[s_index];
		// 	for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
		// 			it != scope->nodes.end(); it++) {
		// 		switch (it->second->type) {
		// 		case NODE_TYPE_NOOP:
		// 			{
		// 				NoopNode* noop_node = (NoopNode*)it->second;
		// 				if (noop_node->experiment != NULL) {
		// 					delete noop_node->experiment;
		// 				}
		// 			}
		// 			break;
		// 		case NODE_TYPE_ACTION:
		// 			{
		// 				ActionNode* action_node = (ActionNode*)it->second;
		// 				if (action_node->experiment != NULL) {
		// 					delete action_node->experiment;
		// 				}
		// 			}
		// 			break;
		// 		case NODE_TYPE_SCOPE:
		// 			{
		// 				ScopeNode* scope_node = (ScopeNode*)it->second;
		// 				if (scope_node->experiment != NULL) {
		// 					delete scope_node->experiment;
		// 				}
		// 			}
		// 			break;
		// 		case NODE_TYPE_BRANCH:
		// 			{
		// 				BranchNode* branch_node = (BranchNode*)it->second;
		// 				if (branch_node->original_experiment != NULL) {
		// 					delete branch_node->original_experiment;
		// 				}
		// 				if (branch_node->branch_experiment != NULL) {
		// 					delete branch_node->branch_experiment;
		// 				}
		// 			}
		// 			break;
		// 		}
		// 	}
		// }

		// wrapper->experiment_iter = 0;

		this->is_new_state = true;
		this->measure_init_networks = init_networks;
		this->measure_new_network = new_network;
		this->sum_vals = 0.0;

		this->state = EXPLORE_EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	} else {
		for (int n_index = 0; n_index < (int)init_networks.size(); n_index++) {
			delete init_networks[n_index];
		}
		delete new_network;

		delete this;

		wrapper->experiment_iter++;
		if (wrapper->experiment_iter >= EXPERIMENT_REFRESH_NUM_ITERS) {
			for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
				Scope* scope = wrapper->solution->scopes[s_index];
				for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
						it != scope->nodes.end(); it++) {
					switch (it->second->type) {
					case NODE_TYPE_NOOP:
						{
							NoopNode* noop_node = (NoopNode*)it->second;
							if (noop_node->experiment != NULL) {
								delete noop_node->experiment;
							}
						}
						break;
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)it->second;
							if (action_node->experiment != NULL) {
								delete action_node->experiment;
							}
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)it->second;
							if (scope_node->experiment != NULL) {
								delete scope_node->experiment;
							}
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)it->second;
							if (branch_node->original_experiment != NULL) {
								delete branch_node->original_experiment;
							}
							if (branch_node->branch_experiment != NULL) {
								delete branch_node->branch_experiment;
							}
						}
						break;
					}
				}
			}

			wrapper->experiment_iter = 0;
		}
	}
}
