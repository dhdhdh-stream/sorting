#include "new_action_experiment.h"

#include <iostream>

#include "absolute_return_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "return_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

const int NEW_ACTION_MIN_NUM_NODES = 3;
const int CREATE_NEW_ACTION_NUM_TRIES = 50;

NewActionExperiment::NewActionExperiment(Scope* scope_context,
										 AbstractNode* node_context,
										 bool is_branch) {
	this->type = EXPERIMENT_TYPE_NEW_ACTION;

	uniform_int_distribution<int> type_distribution(0, 2);
	if (type_distribution(generator) == 0) {
		this->new_action_experiment_type = NEW_ACTION_EXPERIMENT_TYPE_ANY;
	} else {
		this->new_action_experiment_type = NEW_ACTION_EXPERIMENT_TYPE_IN_PLACE;
	}

	this->new_scope = NULL;
	for (int t_index = 0; t_index < CREATE_NEW_ACTION_NUM_TRIES; t_index++) {
		vector<AbstractNode*> possible_starting_nodes;
		scope_context->random_exit_activate(
			scope_context->nodes[0],
			possible_starting_nodes);

		uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
		AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

		geometric_distribution<int> run_distribution(0.33);
		int num_runs = 1 + run_distribution(generator);

		set<AbstractNode*> potential_included_nodes;

		geometric_distribution<int> following_distribution(0.3);
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
		if (num_meaningful_nodes >= NEW_ACTION_MIN_NUM_NODES) {
			this->new_scope = new Scope();

			this->new_scope->node_counter = 0;

			ActionNode* starting_noop_node = new ActionNode();
			starting_noop_node->parent = this->new_scope;
			starting_noop_node->id = this->new_scope->node_counter;
			this->new_scope->node_counter++;
			starting_noop_node->action = Action(ACTION_NOOP, 0);
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
						new_scope_node->index = original_scope_node->index;

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
				case NODE_TYPE_RETURN:
					{
						ReturnNode* original_return_node = (ReturnNode*)(*node_it);

						ReturnNode* new_return_node = new ReturnNode();
						new_return_node->parent = this->new_scope;
						new_return_node->id = this->new_scope->node_counter;
						this->new_scope->node_counter++;
						this->new_scope->nodes[new_return_node->id] = new_return_node;

						node_mappings[original_return_node] = new_return_node;
					}
					break;
				case NODE_TYPE_ABSOLUTE_RETURN:
					{
						AbsoluteReturnNode* original_return_node = (AbsoluteReturnNode*)(*node_it);

						AbsoluteReturnNode* new_return_node = new AbsoluteReturnNode();
						new_return_node->parent = this->new_scope;
						new_return_node->id = this->new_scope->node_counter;
						this->new_scope->node_counter++;
						this->new_scope->nodes[new_return_node->id] = new_return_node;

						new_return_node->location = problem_type->deep_copy_location(original_return_node->location);

						node_mappings[original_return_node] = new_return_node;
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
			new_ending_node->action = Action(ACTION_NOOP, 0);
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

						new_branch_node->analyze_size = original_branch_node->analyze_size;
						new_branch_node->network = new Network(original_branch_node->network);

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
				case NODE_TYPE_RETURN:
					{
						ReturnNode* original_return_node = (ReturnNode*)(*node_it);
						ReturnNode* new_return_node = (ReturnNode*)node_mappings[original_return_node];

						{
							map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
								.find(original_return_node->previous_location);
							if (it == node_mappings.end()) {
								new_return_node->previous_location_id = -1;
								new_return_node->previous_location = NULL;
							} else {
								new_return_node->previous_location_id = it->second->id;
								new_return_node->previous_location = it->second;
							}
						}

						{
							map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
								.find(original_return_node->passed_next_node);
							if (it == node_mappings.end()) {
								new_return_node->passed_next_node_id = new_ending_node->id;
								new_return_node->passed_next_node = new_ending_node;
							} else {
								new_return_node->passed_next_node_id = it->second->id;
								new_return_node->passed_next_node = it->second;
							}
						}
						{
							map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
								.find(original_return_node->skipped_next_node);
							if (it == node_mappings.end()) {
								new_return_node->skipped_next_node_id = new_ending_node->id;
								new_return_node->skipped_next_node = new_ending_node;
							} else {
								new_return_node->skipped_next_node_id = it->second->id;
								new_return_node->skipped_next_node = it->second;
							}
						}
					}
					break;
				case NODE_TYPE_ABSOLUTE_RETURN:
					{
						AbsoluteReturnNode* original_return_node = (AbsoluteReturnNode*)(*node_it);
						AbsoluteReturnNode* new_return_node = (AbsoluteReturnNode*)node_mappings[original_return_node];

						map<AbstractNode*, AbstractNode*>::iterator it = node_mappings
							.find(original_return_node->next_node);
						if (it == node_mappings.end()) {
							new_return_node->next_node_id = new_ending_node->id;
							new_return_node->next_node = new_ending_node;
						} else {
							new_return_node->next_node_id = it->second->id;
							new_return_node->next_node = it->second;
						}
					}
					break;
				}
			}

			new_scope->clean();

			break;
		}
	}

	if (this->new_scope != NULL) {
		this->scope_context = scope_context;

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
		case NODE_TYPE_RETURN:
			{
				ReturnNode* return_node = (ReturnNode*)node_context;
				if (is_branch) {
					random_start_node = return_node->passed_next_node;
				} else {
					random_start_node = return_node->skipped_next_node;
				}
			}
			break;
		case NODE_TYPE_ABSOLUTE_RETURN:
			{
				AbsoluteReturnNode* return_node = (AbsoluteReturnNode*)node_context;
				random_start_node = return_node->next_node;
			}
			break;
		}

		AbstractNode* exit_next_node;
		if (this->new_action_experiment_type == NEW_ACTION_EXPERIMENT_TYPE_IN_PLACE) {
			exit_next_node = random_start_node;
		} else {
			this->scope_context->random_exit_activate(
				random_start_node,
				possible_exits);

			uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
			exit_next_node = possible_exits[exit_distribution(generator)];
		}

		this->test_location_starts.push_back(node_context);
		this->test_location_is_branch.push_back(is_branch);
		this->test_location_exits.push_back(exit_next_node);
		this->test_location_states.push_back(NEW_ACTION_EXPERIMENT_MEASURE_EXISTING);
		this->test_location_existing_scores.push_back(0.0);
		this->test_location_existing_counts.push_back(0);
		this->test_location_existing_truth_counts.push_back(0);
		this->test_location_new_scores.push_back(0.0);
		this->test_location_new_counts.push_back(0);
		this->test_location_new_truth_counts.push_back(0);
		this->test_scope_nodes.push_back(new ScopeNode());
		this->test_scope_nodes.back()->index = this->new_scope->scope_node_index;
		this->new_scope->scope_node_index++;

		this->average_remaining_experiments_from_start = 1.0;

		/**
		 * - added to node_context.experiments outside
		 */

		this->state = NEW_ACTION_EXPERIMENT_STATE_EXPLORE;
		this->generalize_iter = -1;

		this->result = EXPERIMENT_RESULT_NA;
	} else {
		this->result = EXPERIMENT_RESULT_FAIL;
	}
}

NewActionExperiment::~NewActionExperiment() {
	if (this->new_scope != NULL) {
		delete this->new_scope;
	}

	for (int t_index = 0; t_index < (int)this->test_scope_nodes.size(); t_index++) {
		delete this->test_scope_nodes[t_index];
	}

	for (int s_index = 0; s_index < (int)this->successful_scope_nodes.size(); s_index++) {
		/**
		 * - prevent recursive delete
		 */
		this->successful_scope_nodes[s_index]->experiments.clear();

		delete this->successful_scope_nodes[s_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void NewActionExperiment::decrement(AbstractNode* experiment_node) {
	/**
	 * - simply try to remove experiments on new every decrement
	 */
	for (int t_index = (int)this->test_location_starts.size()-1; t_index >= 0; t_index--) {
		if (this->scope_context->nodes.find(this->test_location_starts[t_index]->id) == this->scope_context->nodes.end()) {
			this->test_location_starts.erase(this->test_location_starts.begin() + t_index);
		}
	}
	for (int s_index = (int)this->successful_location_starts.size()-1; s_index >= 0; s_index--) {
		if (this->scope_context->nodes.find(this->successful_location_starts[s_index]->id) == this->scope_context->nodes.end()) {
			this->successful_location_starts.erase(this->successful_location_starts.begin() + s_index);
		}
	}

	bool is_test;
	int location_index;
	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
		if (this->test_location_starts[t_index] == experiment_node) {
			is_test = true;
			location_index = t_index;
			break;
		}
	}
	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
		if (this->successful_location_starts[s_index] == experiment_node) {
			is_test = false;
			location_index = s_index;
			break;
		}
	}

	if (is_test) {
		this->test_location_starts.erase(this->test_location_starts.begin() + location_index);
	} else {
		this->successful_location_starts.erase(this->successful_location_starts.begin() + location_index);
	}

	if (this->test_location_starts.size() == 0
			&& this->successful_location_starts.size() == 0) {
		delete this;
	}
}

NewActionExperimentHistory::NewActionExperimentHistory(
		NewActionExperiment* experiment) {
	this->experiment = experiment;

	this->test_location_index = -1;

	this->instance_count = 0;
}
