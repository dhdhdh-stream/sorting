#include "solution_helpers.h"

#include <iostream>

#include "abstract_experiment.h"
#include "abstract_node.h"
#include "branch_experiment.h"
#include "globals.h"
#include "new_scope_experiment.h"
#include "pass_through_experiment.h"
#include "problem.h"
#include "scope.h"
#include "solution.h"
#include "utilities.h"

using namespace std;

void get_existing_result(Problem* original_problem,
						 RunHelper& run_helper) {
	Problem* copy_problem = original_problem->copy_and_reset();

	#if defined(MDEBUG) && MDEBUG
	run_helper.starting_run_seed = run_index;
	run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
	#endif /* MDEBUG */

	vector<ContextLayer> context;
	solution->scopes[0]->result_activate(
			copy_problem,
			context,
			run_helper);

	if (run_helper.experiments_seen.size() > 0) {
		uniform_int_distribution<int> experiment_distribution(0, run_helper.experiments_seen.size()-1);
		run_helper.experiment_node = run_helper.experiments_seen[experiment_distribution(generator)];

		if (run_helper.experiment_node->experiment == NULL) {
			/**
			 * - don't focus on generalization/reuse
			 *   - may block progress if incompatible spots are grouped together
			 *   - (though may also greatly speed up future progress of course)
			 */
			uniform_int_distribution<int> non_new_distribution(0, 3);
			if (run_helper.experiment_node->parent->nodes.size() > 10
					&& non_new_distribution(generator) != 0) {
				NewScopeExperiment* new_scope_experiment = new NewScopeExperiment(
					run_helper.experiment_node->parent,
					run_helper.experiment_node,
					run_helper.experiment_node->experiment_is_branch);

				if (new_scope_experiment->result == EXPERIMENT_RESULT_FAIL) {
					delete new_scope_experiment;
				} else {
					run_helper.experiment_node->experiment = new_scope_experiment;
				}
			} else {
				/**
				 * - weigh towards PassThroughExperiments as cheaper and potentially just as effective
				 *   - solutions are often made of relatively few distinct decisions, but applied such that has good coverage
				 *     - like tessellation, but have to get both the shape and the pattern correct
				 *       - and PassThroughExperiments help with both
				 */
				uniform_int_distribution<int> pass_through_distribution(0, 3);
				if (pass_through_distribution(generator) == 0) {
					PassThroughExperiment* new_experiment = new PassThroughExperiment(
						run_helper.experiment_node->parent,
						run_helper.experiment_node,
						run_helper.experiment_node->experiment_is_branch);

					run_helper.experiment_node->experiment = new_experiment;
				} else {
					BranchExperiment* new_experiment = new BranchExperiment(
						run_helper.experiment_node->parent,
						run_helper.experiment_node,
						run_helper.experiment_node->experiment_is_branch);

					run_helper.experiment_node->experiment = new_experiment;
				}
			}
		}

		if (run_helper.experiment_node->experiment != NULL) {
			switch (run_helper.experiment_node->experiment->type) {
			case EXPERIMENT_TYPE_BRANCH:
				{
					BranchExperiment* branch_experiment = (BranchExperiment*)run_helper.experiment_node->experiment;
					run_helper.experiment_history = new BranchExperimentHistory(branch_experiment);
				}
				break;
			case EXPERIMENT_TYPE_PASS_THROUGH:
				{
					PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)run_helper.experiment_node->experiment;
					run_helper.experiment_history = new PassThroughExperimentHistory(pass_through_experiment);
				}
				break;
			case EXPERIMENT_TYPE_NEW_SCOPE:
				{
					NewScopeExperiment* new_scope_experiment = (NewScopeExperiment*)run_helper.experiment_node->experiment;
					run_helper.experiment_history = new NewScopeExperimentHistory(new_scope_experiment);
				}
				break;
			}
		}

		double target_val = copy_problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		run_helper.result = target_val;
	}

	delete copy_problem;
}
