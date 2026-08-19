#include "solution_helpers.h"

#include "abstract_node.h"
#include "scope.h"
#include "score_network.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int RUNS_PER_PREDICT = 2;
#else
const int RUNS_PER_PREDICT = 20;
#endif /* MDEBUG */

double predict_helper(AbstractNode* starting_next_node,
					  Eigen::VectorXf& starting_state) {
	Scope* scope = starting_next_node->parent;

	double sum_vals = 0.0;
	for (int r_index = 0; r_index < RUNS_PER_PREDICT; r_index++) {
		Eigen::VectorXf state = starting_state;
		AbstractNode* node_context = starting_next_node;
		while (node_context != NULL) {
			node_context->predict_step(state,
									   node_context);
		}

		scope->end_score_network->activate(state);
		sum_vals += scope->end_score_network->output->acti_vals(0);
	}

	return sum_vals / RUNS_PER_PREDICT;
}
