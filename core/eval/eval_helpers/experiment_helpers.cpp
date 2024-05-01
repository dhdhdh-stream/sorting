#include "eval.h"

#include "eval_experiment.h"
#include "globals.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

void Eval::experiment_activate(Problem* problem,
							   RunHelper& run_helper) {
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

	if (root_history->experiment_history != NULL) {
		EvalExperimentHistory* eval_experiment_history = (EvalExperimentHistory*)root_history->experiment_history;
		EvalExperiment* eval_experiment = (EvalExperiment*)eval_experiment_history->experiment;
		eval_experiment->back_activate(inner_context,
									   eval_experiment_history);
	}

	if (run_helper.experiments_seen_order.size() == 0) {
		if (!run_helper.exceeded_limit) {
			uniform_int_distribution<int> target_distribution(0, 1);
			if (target_distribution(generator) == 0) {
				create_eval_experiment(root_history);
			}
		}
	}

	delete root_history;
}
