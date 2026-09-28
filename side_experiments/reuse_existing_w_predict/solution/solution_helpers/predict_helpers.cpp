#include "solution_helpers.h"

#include "scope_node.h"
#include "score_network.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

double predict_helper(Eigen::VectorXf& state,
					  AbstractNode* exit_next_node,
					  vector<AbstractNode*>& node_context,
					  SolutionWrapper* wrapper) {
	{
		AbstractNode* curr_node = exit_next_node;
		while (curr_node != NULL) {
			curr_node->predict_step(state,
									curr_node);
		}
	}

	for (int l_index = node_context.size()-2; l_index >= 0; l_index--) {
		ScopeNode* scope_node = (ScopeNode*)node_context[l_index];
		AbstractNode* curr_node = scope_node->next_node;
		while (curr_node != NULL) {
			curr_node->predict_step(state,
									curr_node);
		}
	}

	wrapper->solution->score_network->activate(state);

	return wrapper->solution->score_network->output->acti_vals(0);
}
