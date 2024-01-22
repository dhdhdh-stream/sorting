#include "clean_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void CleanExperiment::verify_new_activate(AbstractNode*& curr_node,
										  int& exit_depth,
										  AbstractNode*& exit_node) {
	if (this->clean_exit_depth == 0) {
		curr_node = this->clean_exit_node;
	} else {
		curr_node = NULL;

		exit_depth = this->clean_exit_depth-1;
		exit_node = this->clean_exit_node;
	}
}

void CleanExperiment::verify_new_backprop(double target_val) {
	this->new_score += target_val;

	this->state_iter++;
	if (this->state == CLEAN_EXPERIMENT_STATE_VERIFY_1ST
			&& this->state_iter >= VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints) {
		this->new_score /= (VERIFY_1ST_MULTIPLIER * solution->curr_num_datapoints);

		if (this->new_score >= this->existing_score) {
			this->existing_score = 0.0;
			this->new_score = 0.0;

			this->state = CLEAN_EXPERIMENT_STATE_VERIFY_2ND_EXISTING;
			this->state_iter = 0;
		} else {
			cout << "Clean verify 1st fail" << endl;
			this->state = CLEAN_EXPERIMENT_STATE_FAIL;
		}
	} else if (this->state_iter >= VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints) {
		this->new_score /= (VERIFY_2ND_MULTIPLIER * solution->curr_num_datapoints);

		if (this->new_score >= this->existing_score) {
			cout << "Clean" << endl;
			cout << "verify" << endl;
			cout << "this->scope_context:" << endl;
			for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
				cout << c_index << ": " << this->scope_context[c_index] << endl;
			}
			cout << "this->node_context:" << endl;
			for (int c_index = 0; c_index < (int)this->node_context.size(); c_index++) {
				cout << c_index << ": " << this->node_context[c_index] << endl;
			}
			cout << "this->clean_exit_depth: " << this->clean_exit_depth << endl;
			if (this->clean_exit_node == NULL) {
				cout << "this->clean_exit_node_id: " << -1 << endl;
			} else {
				cout << "this->clean_exit_node_id: " << this->clean_exit_node->id << endl;
			}

			cout << "this->existing_score: " << this->existing_score << endl;
			cout << "this->new_score: " << this->new_score << endl;

			cout << "success" << endl;

			cout << endl;

			Scope* containing_scope = solution->scopes[this->scope_context.back()];

			int new_exit_node_id;
			AbstractNode* new_exit_node;
			if (this->clean_exit_depth != 0) {
				new_exit_node_id = containing_scope->node_counter;
				containing_scope->node_counter++;
				new_exit_node = new ExitNode();
				new_exit_node->parent = containing_scope;
				new_exit_node->id = new_exit_node_id;
				containing_scope->nodes[new_exit_node->id] = new_exit_node;

				((ExitNode*)new_exit_node)->is_exit = false;
				((ExitNode*)new_exit_node)->exit_depth = this->clean_exit_depth;
				((ExitNode*)new_exit_node)->exit_node_parent_id = this->scope_context[this->scope_context.size()-1 - this->clean_exit_depth];
				if (this->clean_exit_node == NULL) {
					((ExitNode*)new_exit_node)->exit_node_id = -1;
				} else {
					((ExitNode*)new_exit_node)->exit_node_id = this->clean_exit_node->id;
				}
				((ExitNode*)new_exit_node)->exit_node = this->clean_exit_node;
			} else {
				if (this->clean_exit_node == NULL) {
					new_exit_node_id = -1;
				} else {
					new_exit_node_id = this->clean_exit_node->id;
				}
				new_exit_node = this->clean_exit_node;
			}

			if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)containing_scope->nodes[this->node_context.back()];

				action_node->next_node_id = new_exit_node_id;
				action_node->next_node = new_exit_node;
			} else if (containing_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)containing_scope->nodes[this->node_context.back()];

				scope_node->next_node_id = new_exit_node_id;
				scope_node->next_node = new_exit_node;
			} else {
				BranchNode* branch_node = (BranchNode*)containing_scope->nodes[this->node_context.back()];

				if (branch_node->experiment_is_branch) {
					branch_node->branch_next_node_id = new_exit_node_id;
					branch_node->branch_next_node = new_exit_node;
				} else {
					branch_node->original_next_node_id = new_exit_node_id;
					branch_node->original_next_node = new_exit_node;
				}
			}

			this->state = CLEAN_EXPERIMENT_STATE_SUCCESS;
		} else {
			cout << "Clean verify 2nd fail" << endl;
			this->state = CLEAN_EXPERIMENT_STATE_FAIL;
		}
	}
}
