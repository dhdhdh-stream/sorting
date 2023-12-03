#include "outer_experiment.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void OuterExperiment::capture_verify_activate(
		Problem& problem,
		RunHelper& run_helper) {
	Problem curr_problem = problem;
	curr_problem.current_world = curr_problem.initial_world;
	curr_problem.current_pointer = 0;
	this->verify_problems[this->state_iter] = curr_problem;
	#if defined(MDEBUG) && MDEBUG
	this->verify_seeds[this->state_iter] = run_helper.starting_run_seed;
	#endif /* MDEBUG */

	vector<ContextLayer> context;
	context.push_back(ContextLayer());

	context.back().scope = NULL;
	context.back().node = NULL;

	// unused
	AbstractNode* curr_node = NULL;
	int exit_depth = -1;
	AbstractNode* exit_node = NULL;

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = new ActionNodeHistory(this->best_actions[s_index]);
			this->best_actions[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				action_node_history);
			delete action_node_history;
		} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			this->best_potential_scopes[s_index]->capture_verify_activate(
				problem,
				context,
				run_helper);
		} else {
			ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->best_root_scope_nodes[s_index]);
			this->best_root_scope_nodes[s_index]->activate(
				curr_node,
				problem,
				context,
				exit_depth,
				exit_node,
				run_helper,
				scope_node_history);
			delete scope_node_history;
		}
	}
}

void OuterExperiment::capture_verify_backprop() {
	this->state_iter++;
	if (this->state_iter >= NUM_VERIFY_SAMPLES) {
		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
				this->best_potential_scopes[s_index]->scope_node_placeholder->verify_key = this;
			}
		}
		solution->verify_key = this;
		solution->verify_problems = this->verify_problems;
		#if defined(MDEBUG) && MDEBUG
		solution->verify_seeds = this->verify_seeds;
		#endif /* MDEBUG */

		cout << "success" << endl;

		Scope* new_root_scope = new Scope();
		new_root_scope->id = solution->scope_counter;
		solution->scope_counter++;
		solution->scopes[new_root_scope->id] = new_root_scope;

		new_root_scope->num_input_states = 0;
		new_root_scope->num_local_states = 0;

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = new_root_scope;
		starting_noop_node->id = 0;
		starting_noop_node->action = Action(ACTION_NOOP);
		new_root_scope->nodes[starting_noop_node->id] = starting_noop_node;

		if (this->best_step_types[0] == STEP_TYPE_ACTION) {
			starting_noop_node->next_node_id = 1;
			starting_noop_node->next_node = this->best_actions[0];
		} else if (this->best_step_types[0] == STEP_TYPE_POTENTIAL_SCOPE) {
			starting_noop_node->next_node_id = 1;
			starting_noop_node->next_node = this->best_potential_scopes[0]->scope_node_placeholder;
		} else {
			starting_noop_node->next_node_id = 1;
			starting_noop_node->next_node = this->best_root_scope_nodes[0];
		}

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			int next_node_id;
			AbstractNode* next_node;
			if (s_index == (int)this->best_step_types.size()-1) {
				next_node_id = -1;
				next_node = NULL;
			} else {
				if (this->best_step_types[s_index+1] == STEP_TYPE_ACTION) {
					next_node_id = this->best_actions[s_index+1]->id;
					next_node = this->best_actions[s_index+1];
				} else if (this->best_step_types[s_index+1] == STEP_TYPE_POTENTIAL_SCOPE) {
					next_node_id = this->best_potential_scopes[s_index+1]->scope_node_placeholder->id;
					next_node = this->best_potential_scopes[s_index+1]->scope_node_placeholder;
				} else {
					next_node_id = this->best_root_scope_nodes[s_index+1]->id;
					next_node = this->best_root_scope_nodes[s_index+1];
				}
			}

			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				this->best_actions[s_index]->parent = new_root_scope;
				new_root_scope->nodes[this->best_actions[s_index]->id] = this->best_actions[s_index];

				this->best_actions[s_index]->next_node_id = next_node_id;
				this->best_actions[s_index]->next_node = next_node;
			} else if (this->best_step_types[s_index] == STEP_TYPE_POTENTIAL_SCOPE) {
				ScopeNode* new_scope_node = this->best_potential_scopes[s_index]->scope_node_placeholder;
				this->best_potential_scopes[s_index]->scope_node_placeholder = NULL;
				new_scope_node->parent = new_root_scope;
				new_root_scope->nodes[new_scope_node->id] = new_scope_node;

				solution->scopes[this->best_potential_scopes[s_index]->scope->id] = this->best_potential_scopes[s_index]->scope;
				new_scope_node->inner_scope = this->best_potential_scopes[s_index]->scope;
				this->best_potential_scopes[s_index]->scope = NULL;

				for (int i_index = 0; i_index < (int)this->best_potential_scopes[s_index]->input_types.size(); i_index++) {
					new_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_scope_node->input_inner_indexes.push_back(this->best_potential_scopes[s_index]->input_inner_indexes[i_index]);
					new_scope_node->input_outer_is_local.push_back(false);
					new_scope_node->input_outer_indexes.push_back(-1);
					new_scope_node->input_init_vals.push_back(this->best_potential_scopes[s_index]->input_init_vals[i_index]);
					new_scope_node->input_init_index_vals.push_back(this->best_potential_scopes[s_index]->input_init_index_vals[i_index]);
				}

				new_scope_node->is_loop = false;
				new_scope_node->continue_score_mod = 0.0;
				new_scope_node->halt_score_mod = 0.0;
				new_scope_node->max_iters = 0;

				new_scope_node->next_node_id = next_node_id;
				new_scope_node->next_node = next_node;

				delete this->best_potential_scopes[s_index];
			} else {
				this->best_root_scope_nodes[s_index]->parent = new_root_scope;
				new_root_scope->nodes[this->best_root_scope_nodes[s_index]->id] = this->best_root_scope_nodes[s_index];

				this->best_root_scope_nodes[s_index]->next_node_id = next_node_id;
				this->best_root_scope_nodes[s_index]->next_node = next_node;
			}
		}
		this->best_actions.clear();
		this->best_potential_scopes.clear();
		this->best_root_scope_nodes.clear();

		new_root_scope->node_counter = 1 + (int)this->best_step_types.size();

		new_root_scope->starting_node_id = 0;
		new_root_scope->starting_node = starting_noop_node;

		solution->root = new_root_scope;

		this->state = OUTER_EXPERIMENT_STATE_SUCCESS;
	}
}
