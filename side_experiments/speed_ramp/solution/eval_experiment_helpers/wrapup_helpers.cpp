#include "eval_experiment.h"

#include <iostream>

#include "abstract_node.h"
#include "constants.h"
#include "helpers.h"
#include "scope.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void EvalExperiment::wrapup_backprop(double target_val,
									 EvalExperimentHistory* history,
									 SolutionWrapper* wrapper,
									 set<Scope*>& updated_scopes) {
	if (history->is_on) {
		this->new_sum_scores += target_val;
		this->new_count++;
	} else {
		this->existing_sum_scores += target_val;
		this->existing_count++;
	}

	this->state_iter++;
	if (this->state_iter >= RAMP_EPOCH_NUM_ITERS) {
		switch (this->result) {
		case EVAL_RESULT_FAIL:
			this->curr_ramp--;
			if (this->curr_ramp < 0) {
				this->node_context->experiment = NULL;
				delete this;
			}
			break;
		case EVAL_RESULT_SUCCESS:
			{
				double existing_score_average = this->existing_sum_scores / (double)this->existing_count;
				double new_score_average = this->new_sum_scores / (double)this->new_count;

				this->existing_sum_scores = 0.0;
				this->existing_count = 0;
				this->new_sum_scores = 0.0;
				this->new_count = 0;

				this->state_iter = 0;

				#if defined(MDEBUG) && MDEBUG
				if (new_score_average >= existing_score_average || rand()%3 != 0) {
				#else
				if (new_score_average >= existing_score_average) {
				#endif /* MDEBUG */
					this->curr_ramp++;
					this->num_fail = 0;

					if (this->curr_ramp >= EXPERIMENT_NUM_GEARS-1) {
						updated_scopes.insert(this->node_context->parent);

						add(wrapper);
						this->node_context->experiment = NULL;
						delete this;

						// temp
						double score_average = wrapper->solution->sum_scores / (double)HISTORIES_NUM_SAVE;
						cout << "score_average: " << score_average << endl;

						int explores_in_flight = 0;
						int evals_in_flight = 0;
						for (map<int, Scope*>::iterator scope_it = wrapper->solution->scopes.begin();
								scope_it != wrapper->solution->scopes.end(); scope_it++) {
							for (map<int, AbstractNode*>::iterator node_it = scope_it->second->nodes.begin();
									node_it != scope_it->second->nodes.end(); node_it++) {
								if (node_it->second->experiment != NULL) {
									switch (node_it->second->experiment->type) {
									case EXPERIMENT_TYPE_EXPLORE:
										explores_in_flight++;
										break;
									case EXPERIMENT_TYPE_EVAL:
										evals_in_flight++;
										break;
									}
								}
							}
						}
						cout << "explores_in_flight: " << explores_in_flight << endl;
						cout << "evals_in_flight: " << evals_in_flight << endl;
					}
				} else {
					this->num_fail++;
					if (this->num_fail >= 2) {
						this->curr_ramp--;
						this->num_fail = 0;

						if (this->curr_ramp < 0) {
							this->node_context->experiment = NULL;
							delete this;
						}
					}
				}
			}
			break;
		}
	}
}
