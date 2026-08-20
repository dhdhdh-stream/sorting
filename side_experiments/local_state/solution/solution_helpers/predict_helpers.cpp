#include "solution_helpers.h"

#include "abstract_node.h"
#include "scope.h"
#include "score_network.h"

using namespace std;

double predict_helper(AbstractNode* starting_next_node,
					  Eigen::VectorXf& starting_state) {
	Scope* scope = starting_next_node->parent;

	Eigen::VectorXf state = starting_state;
	AbstractNode* node_context = starting_next_node;
	while (node_context != NULL) {
		node_context->predict_step(state,
								   node_context);
	}

	scope->end_score_network->activate(state);

	return scope->end_score_network->output->acti_vals(0);
}
