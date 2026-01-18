#include "solution_helpers.h"

#include <iostream>

#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int ATTRIBUTE_NUM_EPOCHS = 40;
const int ATTRIBUTE_EPOCH_NUM_ITERS = 100;

void attribute_helper(ScopeHistory* scope_history,
					  double diff,
					  double& sum_impact,
					  double& sum_errors) {
	vector<AbstractNodeHistory*> node_histories(scope_history->node_histories.size());
	for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
			it != scope_history->node_histories.end(); it++) {
		node_histories[it->second->index] = it->second;
	}

	for (int h_index = 0; h_index < (int)node_histories.size(); h_index++) {
		AbstractNode* node = node_histories[h_index]->node;
		switch (node->type) {
		case NODE_TYPE_SCOPE:
			{
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)node_histories[h_index];
				attribute_helper(scope_node_history->scope_history,
								 diff,
								 sum_impact,
								 sum_errors);
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node;
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)node_histories[h_index];
				if (branch_node_history->is_branch) {
					sum_impact += branch_node->branch_impact;

					branch_node->branch_update_sum_vals += (diff - sum_impact);
					branch_node->branch_update_counts++;
				} else {
					sum_impact += branch_node->original_impact;

					branch_node->original_update_sum_vals += (diff - sum_impact);
					branch_node->original_update_counts++;
				}

				sum_errors += abs(diff - sum_impact);
			}
			break;
		}
	}
}

void update_attribute(SolutionWrapper* wrapper) {
	double sum_vals = 0.0;
	for (int h_index = 0; h_index < (int)wrapper->solution->existing_target_val_histories.size(); h_index++) {
		sum_vals += wrapper->solution->existing_target_val_histories[h_index];
	}
	double val_average = sum_vals / (double)wrapper->solution->existing_target_val_histories.size();

	uniform_int_distribution<int> history_distribution(0, wrapper->solution->existing_scope_histories.size()-1);
	for (int epoch_index = 0; epoch_index < ATTRIBUTE_NUM_EPOCHS; epoch_index++) {
		double sum_errors = 0.0;
		for (int iter_index = 0; iter_index < ATTRIBUTE_EPOCH_NUM_ITERS; iter_index++) {
			int index = history_distribution(generator);

			double diff = wrapper->solution->existing_target_val_histories[index] - val_average;

			double sum_impact = 0.0;
			attribute_helper(wrapper->solution->existing_scope_histories[index],
							 diff,
							 sum_impact,
							 sum_errors);
		}

		cout << "sum_errors: " << sum_errors << endl;

		for (int s_index = 0; s_index < (int)wrapper->solution->scopes.size(); s_index++) {
			Scope* scope = wrapper->solution->scopes[s_index];
			for (map<int, AbstractNode*>::iterator it = scope->nodes.begin();
					it != scope->nodes.end(); it++) {
				if (it->second->type == NODE_TYPE_BRANCH) {
					BranchNode* branch_node = (BranchNode*)it->second;
					branch_node->attribute_update();
				}
			}
		}
	}
}
