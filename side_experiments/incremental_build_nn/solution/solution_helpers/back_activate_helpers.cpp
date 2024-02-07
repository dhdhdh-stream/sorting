#include "solution_helpers.h"

using namespace std;

void gather_possible_experiment_helper(vector<Scope*>& scope_context,
									   vector<AbstractNode*>& node_context,
									   vector<vector<Scope*>>& possible_scope_contexts,
									   vector<vector<AbstractNode*>>& possible_node_contexts,
									   AbstractExperimentHistory* experiment_history) {
	if (experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
		BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)experiment_history;
		BranchExperiment* branch_experiment = (BranchExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
			if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = branch_experiment->best_actions[s_index];

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)branch_experiment_history->step_histories[s_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				gather_possible_helper(scope_context,
									   node_context,
									   possible_scope_contexts,
									   possible_node_contexts,
									   scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}
	} else {
		PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)experiment_history;
		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)pass_through_experiment_history->pre_step_histories.size(); s_index++) {
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = pass_through_experiment->best_actions[s_index];

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)pass_through_experiment_history->pre_step_histories[s_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				gather_possible_helper(scope_context,
									   node_context,
									   possible_scope_contexts,
									   possible_node_contexts,
									   scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}

		if (pass_through_experiment_history->branch_experiment_history != NULL) {
			gather_possible_experiment_helper(scope_context,
											  node_context,
											  possible_scope_contexts,
											  possible_node_contexts,
											  pass_through_experiment_history->branch_experiment_history);
		}

		for (int h_index = 0; h_index < (int)pass_through_experiment_history->post_step_histories.size(); h_index++) {
			int s_index = (int)pass_through_experiment->best_step_types.size() - (int)pass_through_experiment_history->post_step_histories.size() + h_index;
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = pass_through_experiment->best_actions[s_index];

				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);

				node_context.back() = NULL;
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)pass_through_experiment_history->post_step_histories[h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				gather_possible_helper(scope_context,
									   node_context,
									   possible_scope_contexts,
									   possible_node_contexts,
									   scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}
	}
}

void gather_possible_helper(vector<Scope*>& scope_context,
							vector<AbstractNode*>& node_context,
							vector<vector<Scope*>>& possible_scope_contexts,
							vector<vector<AbstractNode*>>& possible_node_contexts,
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

				node_context.back() = NULL;

				if (action_node_history->experiment_history != NULL) {
					gather_possible_experiment_helper(scope_context,
													  node_context,
													  possible_scope_contexts,
													  possible_node_contexts,
													  action_node_history->experiment_history);
				}
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			gather_possible_helper(scope_context,
								   node_context,
								   possible_scope_contexts,
								   possible_node_contexts,
								   scope_node_history->scope_history);

			node_context.back() = NULL;

			if (scope_node_history->experiment_history != NULL) {
				gather_possible_experiment_helper(scope_context,
												  node_context,
												  possible_scope_contexts,
												  possible_node_contexts,
												  scope_node_history->experiment_history);
			}
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;

			node_context.back() = node_history->node;

			possible_scope_contexts.push_back(scope_context);
			possible_node_contexts.push_back(node_context);

			node_context.back() = NULL;

			if (branch_node_history->experiment_history != NULL) {
				gather_possible_experiment_helper(scope_context,
												  node_context,
												  possible_scope_contexts,
												  possible_node_contexts,
												  branch_node_history->experiment_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void input_vals_experiment_helper(vector<Scope*>& scope_context,
								  vector<AbstractNode*>& node_context,
								  vector<double>& input_vals,
								  AbstractExperimentHistory* experiment_history) {
	if (experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
		BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)experiment_history;
		BranchExperiment* branch_experiment = (BranchExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
			if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)branch_experiment_history->step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->back_activate(scope_context,
										   node_context,
										   input_vals,
										   action_node_history);
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)branch_experiment_history->step_histories[s_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				input_vals_helper(scope_context,
								  node_context,
								  input_vals,
								  scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}
	} else {
		PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)experiment_history;
		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)pass_through_experiment_history->pre_step_histories.size(); s_index++) {
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->pre_step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->back_activate(scope_context,
										   node_context,
										   input_vals,
										   action_node_history);
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)pass_through_experiment_history->pre_step_histories[s_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				input_vals_helper(scope_context,
								  node_context,
								  input_vals,
								  scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}

		if (pass_through_experiment_history->branch_experiment_history != NULL) {
			input_vals_experiment_helper(scope_context,
										 node_context,
										 input_vals,
										 pass_through_experiment_history->branch_experiment_history);
		}

		for (int h_index = 0; h_index < (int)pass_through_experiment_history->post_step_histories.size(); h_index++) {
			int s_index = (int)pass_through_experiment->best_step_types.size() - (int)pass_through_experiment_history->post_step_histories.size() + h_index;
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->post_step_histories[h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->back_activate(scope_context,
										   node_context,
										   input_vals,
										   action_node_history);
			} else {
				// STEP_TYPE_EXISTING_SCOPE || STEP_TYPE_POTENTIAL_SCOPE
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)pass_through_experiment_history->post_step_histories[h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node;

				input_vals_helper(scope_context,
								  node_context,
								  input_vals,
								  scope_node_history->scope_history);

				node_context.back() = NULL;
			}
		}
	}
}

void input_vals_helper(vector<Scope*>& scope_context,
					   vector<AbstractNode*>& node_context,
					   vector<double>& input_vals,
					   ScopeHistory* scope_history) {
	scope_context.push_back(scope_history->scope);
	node_context.push_back(NULL);

	for (int h_index = 0; h_index < (int)scope_history->node_histories.size(); h_index++) {
		AbstractNodeHistory* node_history = scope_history->node_histories[h_index];
		if (node_history->node->type == NODE_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = (ActionNodeHistory*)node_history;
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			action_node->back_activate(scope_context,
									   node_context,
									   input_vals,
									   action_node_history);

			if (action_node_history->experiment_history != NULL) {
				input_vals_experiment_helper(scope_context,
											 node_context,
											 input_vals,
											 action_node_history->experiment_history);
			}
		} else if (node_history->node->type == NODE_TYPE_SCOPE) {
			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_history;
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

			node_context.back() = scope_node;

			input_vals_helper(scope_context,
							  node_context,
							  input_vals,
							  scope_node_history->scope_history);

			node_context.back() = NULL;

			if (scope_node_history->experiment_history != NULL) {
				input_vals_experiment_helper(scope_context,
											 node_context,
											 input_vals,
											 scope_node_history->experiment_history);
			}
		} else {
			BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_history;
			BranchNode* branch_node = (BranchNode*)branch_node_history->node;
			branch_node->back_activate(scope_context,
									   node_context,
									   input_vals,
									   branch_node_history);

			if (branch_node_history->experiment_history != NULL) {
				input_vals_experiment_helper(scope_context,
											 node_context,
											 input_vals,
											 branch_node_history->experiment_history);
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}
