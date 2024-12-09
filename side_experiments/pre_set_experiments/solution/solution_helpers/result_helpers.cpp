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
		AbstractExperiment* experiment = run_helper.experiments_seen[experiment_distribution(generator)];

		switch (experiment->type) {
		case EXPERIMENT_TYPE_BRANCH:
			{
				BranchExperiment* branch_experiment = (BranchExperiment*)experiment;
				run_helper.experiment_history = new BranchExperimentHistory(branch_experiment);
			}
			break;
		case EXPERIMENT_TYPE_PASS_THROUGH:
			{
				PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment;
				run_helper.experiment_history = new PassThroughExperimentHistory(pass_through_experiment);
			}
			break;
		case EXPERIMENT_TYPE_NEW_SCOPE:
			{
				NewScopeExperiment* new_scope_experiment = (NewScopeExperiment*)experiment;
				run_helper.experiment_history = new NewScopeExperimentHistory(new_scope_experiment);
			}
			break;
		}

		double target_val = copy_problem->score_result();
		target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
		target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		run_helper.result = target_val;
	} else if (run_helper.nodes_seen.size() != 0) {
		create_experiment(run_helper);
	}

	delete copy_problem;
}
