/**
 * - simply don't predict while remaining predict
 */

#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution_wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int PREDICT_NUM_TRIES = 10;
#else
const int PREDICT_NUM_TRIES = 200;
#endif /* MDEBUG */

double existing_predict_helper(AbstractNode* exit_next_node,
							   vector<AbstractNode*>& possible_exits,
							   SolutionWrapper* wrapper) {
	Scope* scope_context = wrapper->scope_histories.back()->scope;

	Eigen::VectorXf state = wrapper->states.back();

	AbstractNode* curr_node = exit_next_node;

	possible_exits.push_back(curr_node);
	while (curr_node != NULL) {
		curr_node->predict_step(state,
								curr_node);
		possible_exits.push_back(curr_node);
	}

	scope_context->score_network->activate(state);
	return scope_context->score_network->output->acti_vals(0);
}

double predict_helper(vector<AbstractNode*>& remaining_predict,
					  AbstractNode* exit_next_node,
					  SolutionWrapper* wrapper) {
	Scope* scope_context = wrapper->scope_histories.back()->scope;

	Eigen::VectorXf state = wrapper->states.back();

	AbstractNode* curr_node = exit_next_node;

	for (int n_index = 0; n_index < (int)remaining_predict.size(); n_index++) {
		remaining_predict[n_index]->predict_step(
			state,
			curr_node);
	}

	while (curr_node != NULL) {
		curr_node->predict_step(state,
								curr_node);
	}

	scope_context->score_network->activate(state);
	return scope_context->score_network->output->acti_vals(0);
}

void predict_helper(SolutionWrapper* wrapper) {
	Scope* scope_context = wrapper->scope_histories.back()->scope;

	vector<AbstractNode*> possible_exits;
	double existing_predict_val = existing_predict_helper(
		wrapper->node_context.back(),
		possible_exits,
		wrapper);

	vector<AbstractNode*> best_predict;
	double best_predict_val = numeric_limits<double>::lowest();

	geometric_distribution<int> exit_distribution(0.3);
	for (int t_index = 0; t_index < PREDICT_NUM_TRIES; t_index++) {
		int random_index;
		while (true) {
			random_index = exit_distribution(generator);
			if (random_index < (int)possible_exits.size()) {
				break;
			}
		}
		AbstractNode* curr_exit_next_node = possible_exits[random_index];

		vector<AbstractNode*> curr_predict;

		int new_num_steps;
		geometric_distribution<int> geo_distribution(0.3);
		if (random_index == 0) {
			new_num_steps = 1 + geo_distribution(generator);
		} else {
			new_num_steps = geo_distribution(generator);
		}
		uniform_int_distribution<int> action_distribution(0, scope_context->generic_action_nodes.size()-1);
		uniform_int_distribution<int> child_distribution(0, scope_context->generic_scope_nodes.size()-1);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			bool is_scope = false;
			if (scope_context->generic_scope_nodes.size() > 0) {
				if (scope_context->generic_scope_nodes.size() <= RAW_ACTION_WEIGHT) {
					uniform_int_distribution<int> scope_distribution(0, scope_context->generic_scope_nodes.size() + RAW_ACTION_WEIGHT - 1);
					if (scope_distribution(generator) < (int)scope_context->generic_scope_nodes.size()) {
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
				ScopeNode* generic_scope_node = scope_context->generic_scope_nodes[child_distribution(generator)];
				curr_predict.push_back(generic_scope_node);
			} else {
				ActionNode* generic_action_node = scope_context->generic_action_nodes[action_distribution(generator)];
				curr_predict.push_back(generic_action_node);
			}
		}

		double curr_predict_val = predict_helper(curr_predict,
												 curr_exit_next_node,
												 wrapper);

		if (curr_predict_val > best_predict_val) {
			best_predict = curr_predict;
			best_predict_val = curr_predict_val;
		}
	}

	if (best_predict_val > existing_predict_val) {
		wrapper->remaining_predict.back() = best_predict;

		if (wrapper->run_type == RUN_TYPE_UPDATE) {
			wrapper->predict_scope_histories.push_back(wrapper->scope_histories.back());
			wrapper->predict_indexes.push_back(wrapper->scope_histories.back()->node_histories.size()-1);
		}
	}
}
