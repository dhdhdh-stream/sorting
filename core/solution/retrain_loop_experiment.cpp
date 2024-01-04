#include "retrain_loop_experiment.h"

#include "globals.h"
#include "scope.h"
#include "solution.h"
#include "state.h"

using namespace std;

RetrainLoopExperiment::RetrainLoopExperiment(ScopeNode* scope_node) {
	this->type = EXPERIMENT_TYPE_RETRAIN_LOOP;

	this->scope_node = scope_node;

	this->average_remaining_experiments_from_start = 1.0;
	/**
	 * - start with a 50% chance to bypass
	 */
	this->average_instances_per_run = 1.0;

	this->o_target_val_histories.reserve(solution->curr_num_datapoints);

	this->state = RETRAIN_LOOP_EXPERIMENT_STATE_MEASURE_EXISTING;
	this->state_iter = 0;

	this->combined_score = 0.0;
}

RetrainLoopExperiment::~RetrainLoopExperiment() {
	for (int s_index = 0; s_index < (int)this->new_states.size(); s_index++) {
		delete this->new_states[s_index];
	}

	for (int h_index = 0; h_index < (int)this->i_scope_histories.size(); h_index++) {
		delete this->i_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

RetrainLoopExperimentOverallHistory::RetrainLoopExperimentOverallHistory(
		RetrainLoopExperiment* experiment) {
	this->experiment = experiment;

	this->instance_count = 0;

	this->has_target = false;
	this->num_iters = 0;
}
