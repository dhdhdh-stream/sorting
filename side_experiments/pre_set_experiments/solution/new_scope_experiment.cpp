#include "new_scope_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int NEW_SCOPE_MIN_NUM_NODES = 3;
const int CREATE_NEW_SCOPE_NUM_TRIES = 50;

NewScopeExperiment::NewScopeExperiment(Scope* scope_context,
									   AbstractNode* node_context,
									   bool is_branch) {
	this->type = EXPERIMENT_TYPE_NEW_SCOPE;

	this->new_scope = NULL;
	for (int t_index = 0; t_index < CREATE_NEW_SCOPE_NUM_TRIES; t_index++) {
		vector<AbstractNode*> possible_starting_nodes;
		scope_context->random_exit_activate(
			scope_context->nodes[0],
			possible_starting_nodes);

		uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
		AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

		geometric_distribution<int> run_distribution(0.33);
		int num_runs = 1 + run_distribution(generator);

		set<AbstractNode*> potential_included_nodes;

		/**
		 * TODO: scale by possible length
		 */
		geometric_distribution<int> following_distribution(0.2);
		for (int r_index = 0; r_index < num_runs; r_index++) {
			int num_following = 1 + following_distribution(generator);
			scope_context->random_continue(
				potential_starting_node,
				num_following,
				potential_included_nodes);
		}

		int num_meaningful_nodes = 0;
		for (set<AbstractNode*>::iterator it = potential_included_nodes.begin();
				it != potential_included_nodes.end(); it++) {
			switch ((*it)->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)(*it);
					if (action_node->action.move != ACTION_NOOP) {
						num_meaningful_nodes++;
					}
				}
				break;
			case NODE_TYPE_SCOPE:
				num_meaningful_nodes++;
				break;
			}
		}
		if (num_meaningful_nodes >= NEW_SCOPE_MIN_NUM_NODES) {
			this->new_scope = new Scope();
			this->new_scope->id = -1;

			this->new_scope->node_counter = 0;

			ActionNode* starting_noop_node = new ActionNode();
			starting_noop_node->parent = this->new_scope;
			starting_noop_node->id = this->new_scope->node_counter;
			this->new_scope->node_counter++;
			starting_noop_node->action = Action(ACTION_NOOP);
			this->new_scope->nodes[starting_noop_node->id] = starting_noop_node;

			map<AbstractNode*, AbstractNode*> node_mappings;
			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				switch ((*node_it)->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)(*node_it);

						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = this->new_scope;
						new_action_node->id = this->new_scope->node_counter;
						this->new_scope->node_counter++;
						this->new_scope->nodes[new_action_node->id] = new_action_node;

						new_action_node->action = original_action_node->action;

						node_mappings[original_action_node] = new_action_node;
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)(*node_it);

						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = this->new_scope;
						new_scope_node->id = this->new_scope->node_counter;
						this->new_scope->node_counter++;
						this->new_scope->nodes[new_scope_node->id] = new_scope_node;

						new_scope_node->scope = original_scope_node->scope;

						node_mappings[original_scope_node] = new_scope_node;
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* original_branch_node = (BranchNode*)(*node_it);

						BranchNode* new_branch_node = new BranchNode();
						new_branch_node->parent = this->new_scope;
						new_branch_node->id = this->new_scope->node_counter;
						this->new_scope->node_counter++;
						this->new_scope->nodes[new_branch_node->id] = new_branch_node;

						node_mappings[original_branch_node] = new_branch_node;
					}
					break;
				}
			}

			starting_noop_node->next_node_id = node_mappings[potential_starting_node]->id;
			starting_noop_node->next_node = node_mappings[potential_starting_node];

			ActionNode* new_ending_node = new ActionNode();
			new_ending_node->parent = this->new_scope;
			new_ending_node->id = this->new_scope->node_counter;
			this->new_scope->node_counter++;
			new_ending_node->action = Action(ACTION_NOOP);
			this->new_scope->nodes[new_ending_node->id] = new_ending_node;
			new_ending_node->next_node_id = -1;
			new_ending_node->next_node = NULL;

			for (set<AbstractNode*>::iterator node_it = potential_included_nodes.begin();
					node_it != potential_included_nodes.end(); node_it++) {
				switch ((*node_it)->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* original_action_node = (ActionNode*)(*node_it);
						ActionNode* new_action_node = (ActionNode*)node_mappings[original_action_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_action_node->next_node);
						if (it == node_mappings.end()) {
							new_action_node->next_node_id = new_ending_node->id;
							new_action_node->next_node = new_ending_node;
						} else {
							new_action_node->next_node_id = it->second->id;
							new_action_node->next_node = it->second;
						}
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* original_scope_node = (ScopeNode*)(*node_it);
						ScopeNode* new_scope_node = (ScopeNode*)node_mappings[original_scope_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_scope_node->next_node);
						if (it == node_mappings.end()) {
							new_scope_node->next_node_id = new_ending_node->id;
							new_scope_node->next_node = new_ending_node;
						} else {
							new_scope_node->next_node_id = it->second->id;
							new_scope_node->next_node = it->second;
						}
					}
					break;
				case NODE_TYPE_BRANCH:
					{
						BranchNode* original_branch_node = (BranchNode*)(*node_it);
						BranchNode* new_branch_node = (BranchNode*)node_mappings[original_branch_node];

						new_branch_node->network = new Network(original_branch_node->network);
						for (int i_index = (int)original_branch_node->inputs.size()-1; i_index >= 0; i_index--) {
							AbstractNode* original_input_node = scope_context->nodes[
								original_branch_node->inputs[i_index].first.second[0]];
							map<AbstractNode*, AbstractNode*>::iterator it = node_mappings.find(original_input_node);
							if (it == node_mappings.end()) {
								new_branch_node->network->remove_input(i_index);
							} else {
								pair<pair<vector<int>,vector<int>>,int> new_input = original_branch_node->inputs[i_index];
								new_input.first.first[0] = -1;
								new_input.first.second[0] = it->second->id;
								new_branch_node->inputs.insert(new_branch_node->inputs.begin(), new_input);
							}
						}

						map<AbstractNode*, AbstractNode*>::iterator original_it = node_mappings
							.find(original_branch_node->original_next_node);
						if (original_it == node_mappings.end()) {
							new_branch_node->original_next_node_id = new_ending_node->id;
							new_branch_node->original_next_node = new_ending_node;
						} else {
							new_branch_node->original_next_node_id = original_it->second->id;
							new_branch_node->original_next_node = original_it->second;
						}
						map<AbstractNode*, AbstractNode*>::iterator branch_it = node_mappings
							.find(original_branch_node->branch_next_node);
						if (branch_it == node_mappings.end()) {
							new_branch_node->branch_next_node_id = new_ending_node->id;
							new_branch_node->branch_next_node = new_ending_node;
						} else {
							new_branch_node->branch_next_node_id = branch_it->second->id;
							new_branch_node->branch_next_node = branch_it->second;
						}
					}
					break;
				}
			}

			break;
		}
	}

	if (this->new_scope != NULL) {
		this->scope_context = scope_context;
		this->node_context = node_context;
		this->is_branch = is_branch;

		vector<AbstractNode*> possible_exits;

		if (node_context->type == NODE_TYPE_ACTION
				&& ((ActionNode*)node_context)->next_node == NULL) {
			possible_exits.push_back(NULL);
		}

		AbstractNode* random_start_node;
		switch (node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node_context;
				random_start_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)node_context;
				random_start_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					random_start_node = branch_node->branch_next_node;
				} else {
					random_start_node = branch_node->original_next_node;
				}
			}
			break;
		}

		uniform_int_distribution<int> in_place_distribution(0, 1);
		if (in_place_distribution(generator) == 0) {
			this->exit_next_node = random_start_node;
		} else {
			this->scope_context->random_exit_activate(
				random_start_node,
				possible_exits);

			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			this->exit_next_node = possible_exits[exit_distribution(generator)];
		}

		this->state = NEW_SCOPE_EXPERIMENT_STATE_MEASURE;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

NewScopeExperiment::~NewScopeExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

NewScopeExperimentHistory::NewScopeExperimentHistory(
		NewScopeExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;
}
