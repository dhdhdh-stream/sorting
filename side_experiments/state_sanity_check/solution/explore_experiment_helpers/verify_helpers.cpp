#if defined(MDEBUG) && MDEBUG

#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "init_network.h"
#include "noop_node.h"
#include "pass_through_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

const int VERIFY_NUM_ITERS = 10;

void ExploreExperiment::verify_check_activate(
		vector<double>& obs,
		ExploreExperimentHistory* history,
		SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	vector<bool> curr_dependencies_is_hit(this->dependencies.size());
	vector<vector<double>> curr_dependencies_obs(this->dependencies.size());
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		bool is_hit;
		vector<double> obs;
		fetch_dependency_helper(scope_history,
								this->dependencies[d_index],
								0,
								is_hit,
								obs);
		curr_dependencies_is_hit[d_index] = is_hit;
		curr_dependencies_obs[d_index] = obs;
	}

	vector<double> existing_state(NEW_STATE_NUM_ADD, 0.0);
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		if (curr_dependencies_is_hit[d_index]) {
			this->existing_init_networks[d_index]->init_activate(
				existing_state,
				curr_dependencies_obs[d_index]);
		}
	}
	history->verify_original_state_vals.push_back(existing_state);

	vector<double> new_state(NEW_STATE_NUM_ADD, 0.0);
	for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
		if (curr_dependencies_is_hit[d_index]) {
			this->new_init_networks[d_index]->init_activate(
				new_state,
				curr_dependencies_obs[d_index]);
		}
	}
	history->verify_branch_state_vals.push_back(new_state);

	bool is_branch;
	if (wrapper->curr_run_seed%2 == 0) {
		is_branch = true;
	} else {
		is_branch = false;
	}
	wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);

	if (is_branch) {
		ExploreExperimentState* new_experiment_state = new ExploreExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	}
}

void ExploreExperiment::verify_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	if (experiment_state->step_index >= (int)this->best_step_types.size()) {
		wrapper->node_context.back() = this->exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		if (this->best_step_types[experiment_state->step_index] == STEP_TYPE_ACTION) {
			action = this->best_indexes[experiment_state->step_index];
			is_next = true;

			wrapper->run_num_actions++;
		} else {
			Scope* scope = this->scope_context->child_scopes[this->best_indexes[experiment_state->step_index]];
			ScopeHistory* inner_scope_history = new ScopeHistory(scope);
			wrapper->scope_histories.push_back(inner_scope_history);
			wrapper->node_context.push_back(scope->nodes[0]);
			wrapper->experiment_context.push_back(NULL);

			wrapper->states.push_back(Eigen::VectorXf());
			wrapper->states.back().resize(scope->num_states);
			wrapper->states.back().setConstant(0.0);
		}
	}
}

void ExploreExperiment::verify_callback(vector<double>& obs,
										SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context.back();

	experiment_state->step_index++;
}

void ExploreExperiment::verify_exit_step(vector<double>& obs,
										 SolutionWrapper* wrapper) {
	ExploreExperimentState* experiment_state = (ExploreExperimentState*)wrapper->experiment_context[wrapper->experiment_context.size() - 2];

	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->states.pop_back();

	experiment_state->step_index++;
}

void ExploreExperiment::verify_backprop(double target_val,
										ExploreExperimentHistory* history,
										SolutionWrapper* wrapper) {
	this->verify_problems.push_back(wrapper->problem->copy_and_reset());
	this->verify_run_seeds.push_back(wrapper->starting_run_seed);
	this->verify_original_state_vals.insert(this->verify_original_state_vals.end(),
		history->verify_original_state_vals.begin(), history->verify_original_state_vals.end());
	this->verify_branch_state_vals.insert(this->verify_branch_state_vals.end(),
		history->verify_branch_state_vals.begin(), history->verify_branch_state_vals.end());

	this->state_iter++;
	if (this->state_iter >= EXPERIMENT_MEASURE_NUM_DATAPOINTS) {
		set<Scope*> scopes_needed;
		set<ScopeNode*> transitions_needed;
		scopes_needed.insert(this->scope_context);
		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			get_dependency_changes_helper(this->scope_context,
										  this->dependencies[d_index],
										  0,
										  scopes_needed,
										  transitions_needed);
		}

		for (set<ScopeNode*>::iterator it = transitions_needed.begin();
				it != transitions_needed.end(); it++) {
			ScopeNode* scope_node = *it;
			int out_num_states = scope_node->parent->num_states;
			int in_num_states = scope_node->scope->num_states;
			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				PassThroughNetwork* new_in_pass_through_network = new PassThroughNetwork(
					out_num_states + s_index,
					in_num_states + s_index);
				scope_node->in_pass_through_networks.push_back(new_in_pass_through_network);

				PassThroughNetwork* new_out_pass_through_network = new PassThroughNetwork(
					in_num_states + s_index,
					out_num_states + s_index);
				scope_node->out_pass_through_networks.push_back(new_out_pass_through_network);
			}

			cout << "scope_node->parent->id: " << scope_node->parent->id << endl;
			cout << "scope_node->id: " << scope_node->id << endl;
			cout << "out_num_states: " << out_num_states << endl;
			cout << "in_num_states: " << in_num_states << endl;
		}

		for (set<Scope*>::iterator it = scopes_needed.begin();
				it != scopes_needed.end(); it++) {
			Scope* scope = *it;

			scope->num_states += NEW_STATE_NUM_ADD;

			cout << "scope->id: " << scope->id << endl;
			cout << "scope->num_states: " << scope->num_states << endl;
		}

		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			vector<Scope*> init_network_scope_context;
			add_dependency_helper(this->scope_context,
								  init_network_scope_context,
								  this->dependencies[d_index],
								  0,
								  this->existing_init_networks[d_index]);

			// temp
			cout << "this->dependencies[d_index]:";
			for (int l_index = 0; l_index < (int)this->dependencies[d_index].size(); l_index++) {
				cout << " " << this->dependencies[d_index][l_index];
			}
			cout << endl;
			cout << "this->existing_init_networks[d_index]->init_states:";
			for (int s_index = 0; s_index < (int)this->existing_init_networks[d_index]->init_states.size(); s_index++) {
				cout << " " << this->existing_init_networks[d_index]->init_states[s_index];
			}
			cout << endl;
		}
		this->existing_init_networks.clear();

		for (set<ScopeNode*>::iterator it = transitions_needed.begin();
				it != transitions_needed.end(); it++) {
			ScopeNode* scope_node = *it;
			int out_num_states = scope_node->parent->num_states;
			int in_num_states = scope_node->scope->num_states;
			for (int s_index = 0; s_index < NEW_STATE_NUM_ADD; s_index++) {
				PassThroughNetwork* new_in_pass_through_network = new PassThroughNetwork(
					out_num_states + s_index,
					in_num_states + s_index);
				scope_node->in_pass_through_networks.push_back(new_in_pass_through_network);

				PassThroughNetwork* new_out_pass_through_network = new PassThroughNetwork(
					in_num_states + s_index,
					out_num_states + s_index);
				scope_node->out_pass_through_networks.push_back(new_out_pass_through_network);
			}

			cout << "scope_node->parent->id: " << scope_node->parent->id << endl;
			cout << "scope_node->id: " << scope_node->id << endl;
			cout << "out_num_states: " << out_num_states << endl;
			cout << "in_num_states: " << in_num_states << endl;
		}

		for (set<Scope*>::iterator it = scopes_needed.begin();
				it != scopes_needed.end(); it++) {
			Scope* scope = *it;

			scope->num_states += NEW_STATE_NUM_ADD;

			cout << "scope->id: " << scope->id << endl;
			cout << "scope->num_states: " << scope->num_states << endl;
		}

		for (int d_index = 0; d_index < (int)this->dependencies.size(); d_index++) {
			vector<Scope*> init_network_scope_context;
			add_dependency_helper(this->scope_context,
								  init_network_scope_context,
								  this->dependencies[d_index],
								  0,
								  this->new_init_networks[d_index]);
			// temp
			cout << "this->dependencies[d_index]:";
			for (int l_index = 0; l_index < (int)this->dependencies[d_index].size(); l_index++) {
				cout << " " << this->dependencies[d_index][l_index];
			}
			cout << endl;
			cout << "this->new_init_networks[d_index]->init_states:";
			for (int s_index = 0; s_index < (int)this->new_init_networks[d_index]->init_states.size(); s_index++) {
				cout << " " << this->new_init_networks[d_index]->init_states[s_index];
			}
			cout << endl;
		}
		this->new_init_networks.clear();

		add(true,
			wrapper);
	}
}

#endif /* MDEBUG */