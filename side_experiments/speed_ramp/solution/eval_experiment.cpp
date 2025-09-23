#include "eval_experiment.h"

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "obs_node.h"
#include "scope.h"

using namespace std;

EvalExperiment::EvalExperiment() {
	this->type = EXPERIMENT_TYPE_EVAL;

	this->new_network = NULL;
	this->new_scope = NULL;

	this->state = EVAL_EXPERIMENT_STATE_INITIAL;
	this->state_iter = 0;
	this->num_fail = 0;
}

EvalExperiment::~EvalExperiment() {
	if (this->new_network != NULL) {
		delete this->new_network;
	}

	if (this->new_scope != NULL) {
		delete this->new_scope;
	}
}

void EvalExperiment::clean_inputs(Scope* scope,
								  int node_id) {
	for (int i_index = (int)this->new_network_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->new_network_inputs[i_index].scope_context.size(); l_index++) {
			if (this->new_network_inputs[i_index].scope_context[l_index] == scope
					&& this->new_network_inputs[i_index].node_context[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->new_network_inputs.erase(this->new_network_inputs.begin() + i_index);
			this->new_network->remove_input(i_index);
		}
	}
}

void EvalExperiment::replace_obs_node(Scope* scope,
									  int original_node_id,
									  int new_node_id) {
	for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
		if (this->new_network_inputs[i_index].scope_context.back() == scope
				&& this->new_network_inputs[i_index].node_context.back() == original_node_id) {
			this->new_network_inputs[i_index].node_context.back() = new_node_id;
		}
	}
}

EvalExperimentHistory::EvalExperimentHistory(EvalExperiment* experiment) {
	switch (experiment->state) {
	case EVAL_EXPERIMENT_STATE_INITIAL:
		{
			uniform_int_distribution<int> on_distribution(0, 99);
			if (on_distribution(generator) == 0) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	case EVAL_EXPERIMENT_STATE_RAMP:
		{
			uniform_int_distribution<int> on_distribution(0, EXPERIMENT_NUM_GEARS-1);
			if (experiment->curr_ramp >= on_distribution(generator)) {
				this->is_on = true;
			} else {
				this->is_on = false;
			}
		}
		break;
	}
}

EvalExperimentState::EvalExperimentState(EvalExperiment* experiment) {
	this->experiment = experiment;
}
