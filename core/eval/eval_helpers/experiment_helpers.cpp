#include "eval.h"

#include "eval_pass_through_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

void Eval::experiment_activate(Problem* problem,
							   RunHelper& run_helper) {
	if (this->experiment != NULL) {
		if (run_helper.experiment_histories.size() == 0) {
			EvalPassThroughExperiment* eval_pass_through_experiment = (EvalPassThroughExperiment*)this->experiment;
			run_helper.experiment_histories.push_back(new EvalPassThroughExperimentHistory(eval_pass_through_experiment));
			run_helper.experiments_seen_order.push_back(eval_pass_through_experiment);
		}
	}

	bool run_subscope = true;
	if (this->experiment != NULL) {
		EvalPassThroughExperiment* eval_pass_through_experiment = (EvalPassThroughExperiment*)this->experiment;
		if (eval_pass_through_experiment->state == EVAL_PASS_THROUGH_EXPERIMENT_STATE_MEASURE_EXISTING_SCORE) {
			run_subscope = false;
		}
	}
	if (run_subscope) {
		int existing_num_actions_until_random = solution->num_actions_until_random;
		solution->num_actions_until_random = -1;

		vector<ContextLayer> inner_context;
		inner_context.push_back(ContextLayer());

		inner_context.back().scope = this->subscope;
		inner_context.back().node = NULL;

		ScopeHistory* root_history = new ScopeHistory(this->subscope);
		inner_context.back().scope_history = root_history;

		this->subscope->activate(
			problem,
			inner_context,
			run_helper,
			root_history);

		run_helper.num_decisions++;

		if (this->experiment != NULL) {
			EvalPassThroughExperiment* eval_pass_through_experiment = (EvalPassThroughExperiment*)this->experiment;
			eval_pass_through_experiment->back_activate(problem,
														root_history,
														run_helper);
		} else {
			if (!run_helper.exceeded_limit) {
				create_eval_experiment(root_history);
			}
		}

		delete root_history;

		solution->num_actions_until_random = existing_num_actions_until_random;
	}
}
