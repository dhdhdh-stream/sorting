#include "eval.h"

using namespace std;

void Eval::experiment_activate(Problem* problem,
							   RunHelper& run_helper) {
	this->parent_scope->num_instances_until_target--;
	if (this->parent_scope->num_instances_until_target == 0) {
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
			EvalExperiment* eval_experiment = eval_experiment_history->experiment;
			eval_experiment->back_activate(root_history);
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

		uniform_int_distribution<int> until_distribution(0, (int)(this->parent_scope->average_instances_per_run-1.0)/2.0);
		this->parent_scope->num_instances_until_target = 1 + until_distribution(generator);
	}
}
