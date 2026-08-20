#include "predict_experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution_helpers.h"

using namespace std;

void PredictExperiment::explore_helper() {
	this->best_surprise = 0.0;

	uniform_int_distribution<int> sample_distribution(0, this->existing_state_histories.size()-1);
	for (int e_index = 0; e_index < EXPERIMENT_EXPLORE_NUM_DATAPOINTS; e_index++) {
		int index = sample_distribution(generator);

		this->existing_network->activate(this->existing_state_histories[index]);
		double existing_predicted = this->existing_network->output->acti_vals(0);

		vector<int> curr_step_types;
		vector<int> curr_indexes;

		bool exit_is_next;
		switch (this->node_context->type) {
		case NODE_TYPE_NOOP:
			{
				NoopNode* noop_node = (NoopNode*)this->node_context;
				if (this->exit_next_node == noop_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				if (this->exit_next_node == action_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)this->node_context;
				if (this->exit_next_node == scope_node->next_node) {
					exit_is_next = true;
				} else {
					exit_is_next = false;
				}
			}
			break;
		default:
		// case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					if (this->exit_next_node == branch_node->branch_next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				} else {
					if (this->exit_next_node == branch_node->original_next_node) {
						exit_is_next = true;
					} else {
						exit_is_next = false;
					}
				}
			}
			break;
		}

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		/**
		 * - num_steps less than exit length on average to reduce solution size
		 */
		if (exit_is_next) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}

		vector<int> possible_child_indexes;
		for (int c_index = 0; c_index < (int)this->node_context->parent->child_scopes.size(); c_index++) {
			if (this->node_context->parent->child_scopes[c_index]->nodes.size() > 1) {
				possible_child_indexes.push_back(c_index);
			}
		}
		uniform_int_distribution<int> child_index_distribution(0, possible_child_indexes.size()-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool is_scope = false;
			if (possible_child_indexes.size() > 0) {
				if (possible_child_indexes.size() <= RAW_ACTION_WEIGHT) {
					uniform_int_distribution<int> scope_distribution(0, possible_child_indexes.size() + RAW_ACTION_WEIGHT - 1);
					if (scope_distribution(generator) < (int)possible_child_indexes.size()) {
						is_scope = true;
					}
				} else {
					uniform_int_distribution<int> scope_distribution(0, 1);
					if (scope_distribution(generator) == 0) {
						is_scope = true;
					}
				}
			}
			if (is_scope) {
				curr_step_types.push_back(STEP_TYPE_SCOPE);
				int child_index = possible_child_indexes[child_index_distribution(generator)];
				curr_indexes.push_back(child_index);
			} else {
				curr_step_types.push_back(STEP_TYPE_ACTION);
				curr_indexes.push_back(-1);
			}
		}

		double sum_vals = 0.0;
		for (int r_index = 0; r_index < RUNS_PER_PREDICT; r_index++) {
			Eigen::VectorXf state = this->existing_state_histories[index];
			AbstractNode* node_context = this->exit_next_node;

			for (int s_index = 0; s_index < (int)curr_step_types.size(); s_index++) {
				if (curr_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* generic_action_node = this->scope_context->generic_action_nodes[curr_indexes[s_index]];
					generic_action_node->predict_step(state,
													  node_context);
				} else {
					ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[curr_indexes[s_index]];
					generic_scope_node->predict_step(state,
													 node_context);
				}
			}

			sum_vals += predict_helper(node_context,
									   state);
		}

		double curr_predicted = sum_vals / RUNS_PER_PREDICT;

		double curr_surprise = curr_predicted - existing_predicted;
		if (curr_surprise > best_surprise) {
			this->best_surprise = curr_surprise;
			this->best_step_types = curr_step_types;
			this->best_indexes = curr_indexes;
		}
	}
}
