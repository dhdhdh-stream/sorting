#include "seed_experiment_filter.h"

using namespace std;

void SeedExperimentFilter::find_gather_activate(vector<ContextLayer>& context,
												RunHelper& run_helper) {
	vector<Scope*> curr_gather_scope_context;
	vector<AbstractNode*> curr_gather_node_context;
	bool curr_gather_is_branch;
	int curr_gather_exit_depth;
	AbstractNode* curr_gather_exit_node;
	select_gather(curr_gather_scope_context,
				  curr_gather_node_context,
				  curr_gather_is_branch,
				  curr_gather_exit_depth,
				  curr_gather_exit_node,
				  context[context.size() - this->scope_context.size()].scope_history);

	vector<int> curr_gather_step_types;
	vector<ActionNode*> curr_gather_actions;
	vector<ScopeNode*> curr_gather_existing_scopes;
	vector<ScopeNode*> curr_gather_potential_scopes;

	uniform_int_distribution<int> uniform_distribution(0, 2);
	geometric_distribution<int> geometric_distribution(0.5);
	int new_num_steps = 1 + uniform_distribution(generator) + geometric_distribution(generator);

	uniform_int_distribution<int> new_scope_distribution(0, 3);
	uniform_int_distribution<int> random_scope_distribution(0, 3);
	for (int s_index = 0; s_index < new_num_steps; s_index++) {
		ScopeNode* new_scope_node = NULL;
		if (new_scope_distribution(generator) == 0) {
			if (random_scope_distribution(generator) == 0) {
				uniform_int_distribution<int> distribution(0, solution->scopes.size()-1);
				Scope* scope = next(solution->scopes.begin(), distribution(generator))->second;
				new_scope_node = create_scope(scope,
											  run_helper);
			} else {
				new_scope_node = create_scope(this->scope_context[0],
											  run_helper);
			}
		}
		if (new_scope_node != NULL) {
			curr_gather_step_types.push_back(STEP_TYPE_POTENTIAL_SCOPE);
			curr_gather_actions.push_back(NULL);
			curr_gather_existing_scopes.push_back(NULL);

			curr_gather_potential_scopes.push_back(new_scope_node);
		} else {
			ScopeNode* new_existing_scope_node = reuse_existing(problem);
			if (new_existing_scope_node != NULL) {
				curr_gather_step_types.push_back(STEP_TYPE_EXISTING_SCOPE);
				curr_gather_actions.push_back(NULL);

				curr_gather_existing_scopes.push_back(new_existing_scope_node);

				curr_gather_potential_scopes.push_back(NULL);
			} else {
				curr_gather_step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem->random_action();
				curr_gather_actions.push_back(new_action_node);

				curr_gather_existing_scopes.push_back(NULL);
				curr_gather_potential_scopes.push_back(NULL);
			}
		}
	}

	this->parent->curr_gather = new SeedExperimentGather(
		this->parent,
		curr_gather_scope_context,
		curr_gather_node_context,
		curr_gather_is_branch,
		curr_gather_step_types,
		curr_gather_actions,
		curr_gather_existing_scopes,
		curr_gather_potential_scopes,
		curr_gather_exit_depth,
		curr_gather_exit_node);
	curr_gather_node_context.back()->experiments.push_back(this->parent->curr_gather);
}
