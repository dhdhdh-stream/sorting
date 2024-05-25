// #include "new_action_experiment.h"

// #include <iostream>

// #include "action_node.h"
// #include "branch_node.h"
// #include "globals.h"
// #include "info_branch_node.h"
// #include "problem.h"
// #include "scope.h"
// #include "scope_node.h"
// #include "solution.h"

// using namespace std;

// const int NEW_ACTION_MIN_NUM_NODES = 3;
// const int CREATE_NEW_ACTION_NUM_TRIES = 30;

// NewActionExperiment::NewActionExperiment(Scope* scope_context,
// 										 AbstractNode* node_context,
// 										 bool is_branch) {
// 	this->type = EXPERIMENT_TYPE_NEW_ACTION;

// 	this->starting_node = NULL;
// 	for (int t_index = 0; t_index < CREATE_NEW_ACTION_NUM_TRIES; t_index++) {
// 		vector<AbstractNode*> possible_starting_nodes;
// 		scope_context->random_exit_activate(
// 			scope_context->nodes[0],
// 			possible_starting_nodes);

// 		uniform_int_distribution<int> start_distribution(0, possible_starting_nodes.size()-1);
// 		AbstractNode* potential_starting_node = possible_starting_nodes[start_distribution(generator)];

// 		geometric_distribution<int> run_distribution(0.33);
// 		int num_runs = 1 + run_distribution(generator);

// 		set<AbstractNode*> potential_included_nodes;
// 		geometric_distribution<int> following_distribution(0.33);
// 		for (int r_index = 0; r_index < num_runs; r_index++) {
// 			vector<AbstractNode*> possible_nodes;
// 			scope_context->random_exit_activate(
// 				potential_starting_node,
// 				possible_nodes);

// 			int num_following = 1 + following_distribution(generator);
// 			if (1 + num_following > (int)possible_nodes.size()-1) {
// 				num_following = (int)possible_nodes.size()-2;
// 			}
// 			for (int f_index = 0; f_index < num_following; f_index++) {
// 				potential_included_nodes.insert(possible_nodes[1 + f_index]);
// 			}
// 		}

// 		int num_meaningful_nodes = 0;
// 		for (set<AbstractNode*>::iterator it = potential_included_nodes.begin();
// 				it != potential_included_nodes.end(); it++) {
// 			switch ((*it)->type) {
// 			case NODE_TYPE_ACTION:
// 				{
// 					ActionNode* action_node = (ActionNode*)(*it);
// 					if (action_node->action.move != ACTION_NOOP) {
// 						num_meaningful_nodes++;
// 					}
// 				}
// 				break;
// 			case NODE_TYPE_SCOPE:
// 				num_meaningful_nodes++;
// 				break;
// 			}
// 		}
// 		if (potential_included_nodes.size() >= NEW_ACTION_MIN_NUM_NODES) {
// 			this->starting_node = potential_starting_node;
// 			this->included_nodes = potential_included_nodes;

// 			break;
// 		}
// 	}

// 	if (this->starting_node != NULL) {
// 		this->scope_context = scope_context;

// 		vector<AbstractNode*> possible_exits;

// 		if (node_context->type == NODE_TYPE_ACTION
// 				&& ((ActionNode*)node_context)->next_node == NULL) {
// 			possible_exits.push_back(NULL);
// 		}

// 		AbstractNode* starting_node;
// 		switch (node_context->type) {
// 		case NODE_TYPE_ACTION:
// 			{
// 				ActionNode* action_node = (ActionNode*)node_context;
// 				starting_node = action_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_SCOPE:
// 			{
// 				ScopeNode* scope_node = (ScopeNode*)node_context;
// 				starting_node = scope_node->next_node;
// 			}
// 			break;
// 		case NODE_TYPE_BRANCH:
// 			{
// 				BranchNode* branch_node = (BranchNode*)node_context;
// 				if (is_branch) {
// 					starting_node = branch_node->branch_next_node;
// 				} else {
// 					starting_node = branch_node->original_next_node;
// 				}
// 			}
// 			break;
// 		case NODE_TYPE_INFO_BRANCH:
// 			{
// 				InfoBranchNode* info_branch_node = (InfoBranchNode*)node_context;
// 				if (is_branch) {
// 					starting_node = info_branch_node->branch_next_node;
// 				} else {
// 					starting_node = info_branch_node->original_next_node;
// 				}
// 			}
// 			break;
// 		}

// 		this->scope_context->random_exit_activate(
// 			starting_node,
// 			possible_exits);

// 		uniform_int_distribution<int> distribution(0, possible_exits.size()-1);
// 		int random_index = distribution(generator);
// 		AbstractNode* exit_next_node = possible_exits[random_index];

// 		this->test_location_starts.push_back(node_context);
// 		this->test_location_is_branch.push_back(is_branch);
// 		this->test_location_exits.push_back(exit_next_node);
// 		this->test_location_states.push_back(NEW_ACTION_EXPERIMENT_MEASURE_EXISTING);
// 		this->test_location_existing_scores.push_back(0.0);
// 		this->test_location_existing_counts.push_back(0);
// 		this->test_location_new_scores.push_back(0.0);
// 		this->test_location_new_counts.push_back(0);

// 		this->average_remaining_experiments_from_start = 1.0;

// 		/**
// 		 * - already added to node_context.experiments
// 		 */

// 		this->state = NEW_ACTION_EXPERIMENT_STATE_EXPLORE;
// 		this->generalize_iter = -1;

// 		this->result = EXPERIMENT_RESULT_NA;
// 	} else {
// 		this->result = EXPERIMENT_RESULT_FAIL;
// 	}
// }

// NewActionExperiment::~NewActionExperiment() {
// 	#if defined(MDEBUG) && MDEBUG
// 	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
// 		delete this->verify_problems[p_index];
// 	}
// 	#endif /* MDEBUG */
// }

// void NewActionExperiment::decrement(AbstractNode* experiment_node) {
// 	bool is_test;
// 	int location_index;
// 	for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
// 		if (this->test_location_starts[t_index] == experiment_node) {
// 			is_test = true;
// 			location_index = t_index;
// 			break;
// 		}
// 	}
// 	for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
// 		if (this->successful_location_starts[s_index] == experiment_node) {
// 			is_test = false;
// 			location_index = s_index;
// 			break;
// 		}
// 	}

// 	if (is_test) {
// 		this->test_location_starts.erase(this->test_location_starts.begin() + location_index);
// 	} else {
// 		this->successful_location_starts.erase(this->successful_location_starts.begin() + location_index);
// 	}

// 	if (this->test_location_starts.size() == 0
// 			&& this->successful_location_starts.size() == 0) {
// 		delete this;
// 	}
// }

// NewActionExperimentHistory::NewActionExperimentHistory(
// 		NewActionExperiment* experiment) {
// 	this->experiment = experiment;

// 	this->test_location_index = -1;
// }
