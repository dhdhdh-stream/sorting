#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment_helper(vector<Scope*>& scope_context,
							  vector<AbstractNode*>& node_context,
							  vector<vector<Scope*>>& possible_scope_contexts,
							  vector<vector<AbstractNode*>>& possible_node_contexts,
							  vector<bool>& possible_is_branch,
							  vector<int>& possible_throw_id,
							  ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			if (h_index == 0 || action_node->action.move != ACTION_NOOP) {
				node_context.back() = action_node;

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_is_branch.push_back(false);
				possible_throw_id.push_back(-1);

				node_context.back() = NULL;
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			create_experiment_helper(scope_context,
									 node_context,
									 possible_scope_contexts,
									 possible_node_contexts,
									 possible_is_branch,
									 possible_throw_id,
									 scope_node_history->scope_history);

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(false);
			possible_throw_id.push_back(scope_node_history->throw_id);

			node_context.back() = NULL;
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);
			possible_is_branch.push_back(branch_node_history->is_branch);
			possible_throw_id.push_back(-1);

			node_context.back() = NULL;
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void create_experiment(ScopeHistory* root_history) {
	vector<vector<Scope*>> possible_scope_contexts;
	vector<vector<AbstractNode*>> possible_node_contexts;
	vector<bool> possible_is_branch;
	vector<int> possible_throw_id;

	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	create_experiment_helper(scope_context,
							 node_context,
							 possible_scope_contexts,
							 possible_node_contexts,
							 possible_is_branch,
							 possible_throw_id,
							 root_history);

	uniform_int_distribution<int> possible_distribution(0, (int)possible_scope_contexts.size()-1);
	int rand_index = possible_distribution(generator);

	if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_ACTION) {
		uniform_int_distribution<int> next_distribution(0, 1);
		int context_size = 1;
		while (true) {
			if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
				context_size++;
			} else {
				break;
			}
		}
		/**
		 * - minimize context to generalize/maximize impact
		 */

		uniform_int_distribution<int> pass_through_distribution(0, 1);
		if (pass_through_distribution(generator) == 0) {
			PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				false,
				-1,
				NULL);

			ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
			action_node->experiments.push_back(new_pass_through_experiment);
		} else {
			BranchExperiment* new_branch_experiment = new BranchExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				false,
				-1,
				NULL,
				false);

			ActionNode* action_node = (ActionNode*)possible_node_contexts[rand_index].back();
			action_node->experiments.push_back(new_branch_experiment);
		}
	} else if (possible_node_contexts[rand_index].back()->type == NODE_TYPE_SCOPE) {
		uniform_int_distribution<int> next_distribution(0, 1);
		int context_size = 1;
		while (true) {
			if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
				context_size++;
			} else {
				break;
			}
		}

		uniform_int_distribution<int> pass_through_distribution(0, 1);
		if (pass_through_distribution(generator) == 0) {
			PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				false,
				possible_throw_id[rand_index],
				NULL);

			ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
			scope_node->experiments.push_back(new_pass_through_experiment);
		} else {
			BranchExperiment* new_branch_experiment = new BranchExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				false,
				possible_throw_id[rand_index],
				NULL,
				false);

			ScopeNode* scope_node = (ScopeNode*)possible_node_contexts[rand_index].back();
			scope_node->experiments.push_back(new_branch_experiment);
		}
	} else {
		uniform_int_distribution<int> next_distribution(0, 1);
		int context_size = 1;
		while (true) {
			if (context_size < (int)possible_scope_contexts[rand_index].size() && next_distribution(generator)) {
				context_size++;
			} else {
				break;
			}
		}

		uniform_int_distribution<int> pass_through_distribution(0, 1);
		if (pass_through_distribution(generator) == 0) {
			PassThroughExperiment* new_pass_through_experiment = new PassThroughExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				possible_is_branch[rand_index],
				-1,
				NULL);

			BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
			branch_node->experiments.push_back(new_pass_through_experiment);
			if (possible_is_branch[rand_index]) {
				branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
			} else {
				branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
			}
		} else {
			BranchExperiment* new_branch_experiment = new BranchExperiment(
				vector<Scope*>(possible_scope_contexts[rand_index].end() - context_size, possible_scope_contexts[rand_index].end()),
				vector<AbstractNode*>(possible_node_contexts[rand_index].end() - context_size, possible_node_contexts[rand_index].end()),
				possible_is_branch[rand_index],
				-1,
				NULL,
				false);

			BranchNode* branch_node = (BranchNode*)possible_node_contexts[rand_index].back();
			branch_node->experiments.push_back(new_branch_experiment);
			if (possible_is_branch[rand_index]) {
				branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
			} else {
				branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
			}
		}
	}
}

string getline_helper(std::ifstream& input_file) {
	while (true) {
		string line;
		getline(input_file, line);
		if (line[0] != '#') {
			return line;
		}
	}
}

/**
 * - for now, BranchExperiment with only actions
 */
AbstractExperiment* create_experiment(ifstream& input_file) {
	vector<Scope*> scope_context;
	vector<AbstractNode*> node_context;
	string context_size_line = getline_helper(input_file);
	int context_size = stoi(context_size_line);
	for (int c_index = 0; c_index < context_size; c_index++) {
		string scope_id_line = getline_helper(input_file);
		Scope* scope = solution->scopes[stoi(scope_id_line)];
		scope_context.push_back(scope);

		string node_id_line = getline_helper(input_file);
		AbstractNode* node = scope->nodes[stoi(node_id_line)];
		node_context.push_back(node);
	}

	string is_branch_line = getline_helper(input_file);
	bool is_branch = stoi(is_branch_line);

	string throw_id_line = getline_helper(input_file);
	int throw_id = stoi(throw_id_line);

	vector<int> best_step_types;
	vector<ActionNode*> best_actions;
	vector<ScopeNode*> best_existing_scopes;
	vector<ScopeNode*> best_potential_scopes;
	vector<set<int>> best_catch_throw_ids;
	string num_steps_line = getline_helper(input_file);
	int num_steps = stoi(num_steps_line);
	for (int s_index = 0; s_index < num_steps; s_index++) {
		best_step_types.push_back(STEP_TYPE_ACTION);

		string action_line = getline_helper(input_file);
		ActionNode* new_action_node = new ActionNode();
		new_action_node->action = Action(stoi(action_line));
		best_actions.push_back(new_action_node);

		best_existing_scopes.push_back(NULL);
		best_potential_scopes.push_back(NULL);
		best_catch_throw_ids.push_back(set<int>());
	}

	string exit_depth_line = getline_helper(input_file);
	int exit_depth = stoi(exit_depth_line);

	string exit_next_node_id_line = getline_helper(input_file);
	int exit_next_node_id = stoi(exit_next_node_id_line);
	AbstractNode* exit_next_node;
	if (exit_next_node_id == -1) {
		exit_next_node = NULL;
	} else {
		exit_next_node = scope_context[scope_context.size()-1 - exit_depth]->nodes[exit_next_node_id];
	}

	string exit_throw_id_line = getline_helper(input_file);
	int exit_throw_id = stoi(exit_throw_id_line);

	BranchExperiment* new_branch_experiment = new BranchExperiment(
		scope_context,
		node_context,
		is_branch,
		throw_id,
		NULL,
		true);

	new_branch_experiment->best_step_types = best_step_types;
	new_branch_experiment->best_actions = best_actions;
	new_branch_experiment->best_existing_scopes = best_existing_scopes;
	new_branch_experiment->best_potential_scopes = best_potential_scopes;
	new_branch_experiment->best_catch_throw_ids = best_catch_throw_ids;
	new_branch_experiment->best_exit_depth = exit_depth;
	new_branch_experiment->best_exit_next_node = exit_next_node;
	new_branch_experiment->best_exit_throw_id = exit_throw_id;

	if (node_context.back()->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)node_context.back();
		action_node->experiments.push_back(new_branch_experiment);
	} else if (node_context.back()->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)node_context.back();
		scope_node->experiments.push_back(new_branch_experiment);
	} else {
		BranchNode* branch_node = (BranchNode*)node_context.back();
		branch_node->experiments.push_back(new_branch_experiment);
		if (is_branch) {
			branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_BRANCH);
		} else {
			branch_node->experiment_types.push_back(BRANCH_NODE_EXPERIMENT_TYPE_ORIGINAL);
		}
	}

	return new_branch_experiment;
}
