#if defined(MDEBUG) && MDEBUG

#include "seed_experiment_filter.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "seed_experiment.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

void SeedExperimentFilter::candidate_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->parent->verify_problems[this->parent->state_iter] == NULL) {
		this->parent->verify_problems[this->parent->state_iter] = problem->copy_and_reset();
	}
	this->parent->verify_seeds[this->parent->state_iter] = run_helper.starting_run_seed;

	vector<double> input_vals(this->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_scope_contexts[i_index].size() > 0) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->input_node_contexts[i_index]);
			}
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		if (this->input_scope_contexts[i_index].size() > 0) {
			if (this->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}
	}

	vector<vector<double>> network_input_vals(this->network_input_indexes.size());
	for (int i_index = 0; i_index < (int)this->network_input_indexes.size(); i_index++) {
		network_input_vals[i_index] = vector<double>(this->network_input_indexes[i_index].size());
		for (int v_index = 0; v_index < (int)this->network_input_indexes[i_index].size(); v_index++) {
			network_input_vals[i_index][v_index] = input_vals[this->network_input_indexes[i_index][v_index]];
		}
	}
	this->network->activate(network_input_vals);

	this->branch_node->verify_original_scores.push_back(0.5 + this->network->output->acti_vals[0]);
	this->branch_node->verify_branch_scores.push_back(FILTER_FINAL_CONFIDENCE_THRESHOLD);

	// cout << "context scope" << endl;
	// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
	// 	cout << c_index << ": " << context[c_index].scope->id << endl;
	// }
	// cout << "context node" << endl;
	// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
	// 	if (context[c_index].node == NULL) {
	// 		cout << c_index << ": -1" << endl;
	// 	} else {
	// 		cout << c_index << ": " << context[c_index].node->id << endl;
	// 	}
	// }
	cout << "input_vals:" << endl;
	for (int i_index = 0; i_index < (int)this->input_scope_contexts.size(); i_index++) {
		cout << input_vals[i_index] << endl;
	}
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (decision_is_branch) {
		for (int s_index = 0; s_index < (int)this->filter_step_types.size(); s_index++) {
			if (this->filter_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = new ActionNodeHistory(this->filter_actions[s_index]);
				this->filter_actions[s_index]->activate(curr_node,
														problem,
														context,
														exit_depth,
														exit_node,
														run_helper,
														action_node_history);
				delete action_node_history;
			} else if (this->filter_step_types[s_index] == STEP_TYPE_EXISTING_SCOPE) {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_existing_scopes[s_index]);
				this->filter_existing_scopes[s_index]->activate(curr_node,
																problem,
																context,
																exit_depth,
																exit_node,
																run_helper,
																scope_node_history);
				delete scope_node_history;
			} else {
				ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(this->filter_potential_scopes[s_index]);
				this->filter_potential_scopes[s_index]->activate(curr_node,
																 problem,
																 context,
																 exit_depth,
																 exit_node,
																 run_helper,
																 scope_node_history);
				delete scope_node_history;
			}
		}

		if (this->filter_exit_depth == 0) {
			curr_node = this->filter_exit_next_node;
		} else {
			exit_depth = this->filter_exit_depth-1;
			exit_node = this->filter_exit_next_node;
		}
	} else {
		curr_node = this->seed_next_node;
	}
}

void SeedExperimentFilter::non_candidate_capture_verify_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		AbstractNode*& exit_node,
		RunHelper& run_helper) {
	if (this->parent->verify_problems[this->parent->state_iter] == NULL) {
		this->parent->verify_problems[this->parent->state_iter] = problem->copy_and_reset();
	}
	this->parent->verify_seeds[this->parent->state_iter] = run_helper.starting_run_seed;

	BranchNodeHistory* history = new BranchNodeHistory(this->branch_node);
	context.back().scope_history->node_histories.push_back(history);

	vector<double> input_vals(this->branch_node->input_scope_contexts.size(), 0.0);
	for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
		if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
			if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
				action_node->hook_indexes.push_back(i_index);
				action_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
				action_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
			} else {
				BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
				branch_node->hook_indexes.push_back(i_index);
				branch_node->hook_scope_contexts.push_back(this->branch_node->input_scope_contexts[i_index]);
				branch_node->hook_node_contexts.push_back(this->branch_node->input_node_contexts[i_index]);
			}
		}
	}
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	input_vals_helper(scope_context,
					  node_context,
					  input_vals,
					  context[context.size() - this->branch_node->scope_context.size()].scope_history);
	for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
		if (this->branch_node->input_scope_contexts[i_index].size() > 0) {
			if (this->branch_node->input_node_contexts[i_index].back()->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->branch_node->input_node_contexts[i_index].back();
				action_node->hook_indexes.clear();
				action_node->hook_scope_contexts.clear();
				action_node->hook_node_contexts.clear();
			} else {
				BranchNode* branch_node = (BranchNode*)this->branch_node->input_node_contexts[i_index].back();
				branch_node->hook_indexes.clear();
				branch_node->hook_scope_contexts.clear();
				branch_node->hook_node_contexts.clear();
			}
		}
	}

	double original_score = this->branch_node->original_average_score;
	for (int i_index = 0; i_index < (int)this->branch_node->linear_original_input_indexes.size(); i_index++) {
		original_score += input_vals[this->branch_node->linear_original_input_indexes[i_index]] * this->branch_node->linear_original_weights[i_index];
	}
	if (this->branch_node->original_network != NULL) {
		vector<vector<double>> original_network_input_vals(this->branch_node->original_network_input_indexes.size());
		for (int i_index = 0; i_index < (int)this->branch_node->original_network_input_indexes.size(); i_index++) {
			original_network_input_vals[i_index] = vector<double>(this->branch_node->original_network_input_indexes[i_index].size());
			for (int v_index = 0; v_index < (int)this->branch_node->original_network_input_indexes[i_index].size(); v_index++) {
				original_network_input_vals[i_index][v_index] = input_vals[this->branch_node->original_network_input_indexes[i_index][v_index]];
			}
		}
		this->branch_node->original_network->activate(original_network_input_vals);
		original_score += this->branch_node->original_network->output->acti_vals[0];
	}

	this->branch_node->verify_original_scores.push_back(original_score);
	this->branch_node->verify_branch_scores.push_back(FILTER_CONFIDENCE_THRESHOLD);

	// cout << "context scope" << endl;
	// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
	// 	cout << c_index << ": " << context[c_index].scope->id << endl;
	// }
	// cout << "context node" << endl;
	// for (int c_index = 0; c_index < (int)context.size(); c_index++) {
	// 	if (context[c_index].node == NULL) {
	// 		cout << c_index << ": -1" << endl;
	// 	} else {
	// 		cout << c_index << ": " << context[c_index].node->id << endl;
	// 	}
	// }
	cout << "input_vals:" << endl;
	for (int i_index = 0; i_index < (int)this->branch_node->input_scope_contexts.size(); i_index++) {
		cout << input_vals[i_index] << endl;
	}
	cout << "run_helper.curr_run_seed: " << run_helper.curr_run_seed << endl;
	problem->print();

	if (run_helper.curr_run_seed%2 == 0) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);

	if (history->is_branch) {
		curr_node = this->branch_node->branch_next_node;

		for (int e_index = 0; e_index < (int)this->branch_node->experiments.size(); e_index++) {
			if (this->branch_node->experiment_types[e_index] == BRANCH_NODE_EXPERIMENT_TYPE_BRANCH) {
				bool is_selected = this->branch_node->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					history->experiment_history);
				if (is_selected) {
					return;
				}
			}
		}
	} else {
		curr_node = this->branch_node->original_next_node;

		for (int e_index = 0; e_index < (int)this->branch_node->experiments.size(); e_index++) {
			if (this->branch_node->experiment_types[e_index] == BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL) {
				bool is_selected = this->branch_node->experiments[e_index]->activate(
					curr_node,
					problem,
					context,
					exit_depth,
					exit_node,
					run_helper,
					history->experiment_history);
				if (is_selected) {
					return;
				}
			}
		}
	}
}

#endif /* MDEBUG */