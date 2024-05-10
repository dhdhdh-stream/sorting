#include "eval.h"

#include "eval_pass_through_experiment.h"
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
}
