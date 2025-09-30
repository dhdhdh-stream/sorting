#include "eval_experiment.h"

#include "abstract_node.h"
#include "constants.h"
#include "helpers.h"

using namespace std;

void EvalExperiment::wrapup_backprop(SolutionWrapper* wrapper,
									 set<Scope*>& updated_scopes) {
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
			this->curr_ramp++;
			if (this->curr_ramp >= EXPERIMENT_NUM_GEARS-1) {
				updated_scopes.insert(this->node_context->parent);

				add(wrapper);
				this->node_context->experiment = NULL;
				delete this;

				measure_score(wrapper);
			}
			break;
		}
	}
}
