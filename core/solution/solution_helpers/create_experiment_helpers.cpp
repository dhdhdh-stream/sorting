#include "solution_helpers.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "info_branch_node.h"
#include "info_pass_through_experiment.h"
#include "info_scope.h"
#include "info_scope_node.h"
#include "new_action_experiment.h"
#include "new_info_experiment.h"
#include "pass_through_experiment.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void create_experiment(AbstractNode* explore_node,
					   bool explore_is_branch) {
	// if (explore_node->parent->parent_info_scope == NULL) {
		uniform_int_distribution<int> expensive_distribution(0, 9);
		if (expensive_distribution(generator) == 0) {
			uniform_int_distribution<int> type_distribution(0, 1);
			switch (type_distribution(generator)) {
			case 0:
				{
					// NewInfoExperiment* new_experiment = new NewInfoExperiment(
					// 	possible_scope_contexts[rand_index],
					// 	possible_node_contexts[rand_index],
					// 	possible_is_branch[rand_index],
					// 	NULL);

					// possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
				}
				break;
			case 1:
				{
					BranchExperiment* new_experiment = new BranchExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch,
						NULL);

					explore_node->experiments.push_back(new_experiment);
				}
				break;
			}
		} else {
			uniform_int_distribution<int> pass_through_distribution(0, 3);
			if (pass_through_distribution(generator) != 0) {
				#if defined(MDEBUG) && MDEBUG
				if (explore_node->parent->nodes.size() > 10) {
				#else
				if (explore_node->parent->nodes.size() > 20) {
				#endif /* MDEBUG */
					NewActionExperiment* new_action_experiment = new NewActionExperiment(
						explore_node->parent,
						explore_node,
						explore_is_branch);

					if (new_action_experiment->result == EXPERIMENT_RESULT_FAIL) {
						delete new_action_experiment;
					} else {
						explore_node->experiments.push_back(new_action_experiment);
					}
				}
			} else {
				// PassThroughExperiment* new_experiment = new PassThroughExperiment(
				// 	possible_scope_contexts[rand_index],
				// 	possible_node_contexts[rand_index],
				// 	possible_is_branch[rand_index],
				// 	NULL);

				// possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
			}
		}
	// } else {
	// 	InfoPassThroughExperiment* new_experiment = new InfoPassThroughExperiment(
	// 		possible_info_scope_contexts[rand_index],
	// 		possible_scope_contexts[rand_index],
	// 		possible_node_contexts[rand_index],
	// 		possible_is_branch[rand_index]);

	// 	possible_info_scope_contexts[rand_index]->experiment = new_experiment;
	// 	possible_node_contexts[rand_index]->experiments.push_back(new_experiment);
	// }
}
