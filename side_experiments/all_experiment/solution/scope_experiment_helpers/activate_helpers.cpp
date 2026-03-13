#include "scope_experiment.h"

#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

const int CHECK_ITERS_I0 = 1;
const int CHECK_ITERS_I1 = 20;
const int CHECK_ITERS_I2 = 200;
const int CHECK_ITERS_I3 = 1000;
const int CHECK_ITERS_I4 = 4000;

void ScopeExperiment::check_activate(AbstractNode* experiment_node,
									 vector<double>& obs,
									 SolutionWrapper* wrapper) {
	ScopeExperimentHistory* history = (ScopeExperimentHistory*)wrapper->scope_experiment_history;
	history->is_hit = true;

	ScopeExperimentState* new_experiment_state = new ScopeExperimentState(this);
	wrapper->experiment_context.back() = new_experiment_state;
}

void ScopeExperiment::experiment_step(vector<double>& obs,
									  int& action,
									  bool& is_next,
									  bool& fetch_action,
									  SolutionWrapper* wrapper) {
	ScopeHistory* inner_scope_history = new ScopeHistory(this->new_scope);
	wrapper->scope_histories.push_back(inner_scope_history);
	wrapper->node_context.push_back(this->new_scope->nodes[0]);
	wrapper->experiment_context.push_back(NULL);
}

void ScopeExperiment::set_action(int action,
								 SolutionWrapper* wrapper) {
	// do nothing
}

void ScopeExperiment::experiment_exit_step(SolutionWrapper* wrapper) {
	delete wrapper->scope_histories.back();

	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	wrapper->node_context.back() = this->exit_next_node;

	delete wrapper->experiment_context.back();
	wrapper->experiment_context.back() = NULL;
}

void ScopeExperiment::backprop(double target_val,
							   SolutionWrapper* wrapper,
							   set<Scope*>& updated_scopes) {
	this->total_sum_scores += target_val;
	this->total_count++;

	ScopeExperimentHistory* history = (ScopeExperimentHistory*)wrapper->scope_experiment_history;
	if (history->is_hit) {
		this->sum_scores += target_val;
		this->count++;

		if (this->count == CHECK_ITERS_I0
				|| this->count == CHECK_ITERS_I1
				|| this->count == CHECK_ITERS_I2
				|| this->count == CHECK_ITERS_I3) {
			double average_score = this->sum_scores / this->count;
			if (average_score < this->node_context->val_average) {
				this->node_context->experiment = NULL;
				wrapper->curr_scope_experiment = NULL;
				delete this;
			}
		} else if (this->count == CHECK_ITERS_I4) {
			double average_score = this->sum_scores / this->count;
			if (average_score >= this->node_context->val_average) {
				this->local_improvement = average_score - this->node_context->val_average;
				this->global_improvement = this->local_improvement / this->node_context->average_hits_per_run;

				bool is_success = false;
				if (wrapper->solution->last_scores.size() >= MIN_NUM_LAST_TRACK) {
					int num_better_than = 0;
					for (list<double>::iterator it = wrapper->solution->last_scores.begin();
							it != wrapper->solution->last_scores.end(); it++) {
						if (this->global_improvement >= *it) {
							num_better_than++;
						}
					}

					double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->last_scores.size();

					if (num_better_than >= target_better_than) {
						is_success = true;
					}

					if (wrapper->solution->last_scores.size() >= NUM_LAST_TRACK) {
						wrapper->solution->last_scores.pop_front();
					}
					wrapper->solution->last_scores.push_back(this->global_improvement);
				} else {
					wrapper->solution->last_scores.push_back(this->global_improvement);
				}

				if (is_success) {
					updated_scopes.insert(this->node_context->parent);

					this->node_context->experiment = NULL;
					wrapper->curr_scope_experiment = NULL;

					add(wrapper);

					delete this;
				} else {
					this->node_context->experiment = NULL;
					wrapper->curr_scope_experiment = NULL;
					delete this;
				}
			} else {
				this->node_context->experiment = NULL;
				wrapper->curr_scope_experiment = NULL;
				delete this;
			}
		}
	}
}
