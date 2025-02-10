#include "commit_experiment.h"

#include "branch_experiment.h"

using namespace std;

CommitExperiment::CommitExperiment(Scope* scope_context,
								   AbstractNode* node_context,
								   bool is_branch) {
	this->type = EXPERIMENT_TYPE_COMMIT;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->state = COMMIT_EXPERIMENT_STATE_EXISTING_GATHER;
	this->state_iter = 0;

	this->best_experiment = NULL;
	this->curr_experiment = NULL;

	this->result = EXPERIMENT_RESULT_NA;
}

CommitExperiment::~CommitExperiment() {
	if (this->best_experiment != NULL) {
		delete this->best_experiment;
	}
	if (this->curr_experiment != NULL) {
		delete this->curr_experiment;
	}
}

void CommitExperiment::decrement(AbstractNode* experiment_node) {
	delete this;
}

void CommitExperiment::clean_inputs(Scope* scope,
									int node_id) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].first.first.size(); l_index++) {
			if (this->existing_inputs[i_index].first.first[l_index] == scope
					&& this->existing_inputs[i_index].first.second[l_index] == node_id) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->existing_inputs.erase(this->existing_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
				this->input_histories[h_index].erase(this->input_histories[h_index].begin() + i_index);
			}
		}
	}

	if (scope == this->scope_context) {
		for (int f_index = (int)this->existing_factor_ids.size()-1; f_index >= 0; f_index--) {
			if (this->existing_factor_ids[f_index].first == node_id) {
				this->existing_factor_ids.erase(this->existing_factor_ids.begin() + f_index);
				for (int h_index = 0; h_index < (int)this->factor_histories.size(); h_index++) {
					this->factor_histories[h_index].erase(this->factor_histories[h_index].begin() + f_index);
				}
			}
		}
	}
}

void CommitExperiment::clean_inputs(Scope* scope) {
	for (int i_index = (int)this->existing_inputs.size()-1; i_index >= 0; i_index--) {
		bool is_match = false;
		for (int l_index = 0; l_index < (int)this->existing_inputs[i_index].first.first.size(); l_index++) {
			if (this->existing_inputs[i_index].first.first[l_index] == scope) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			this->existing_inputs.erase(this->existing_inputs.begin() + i_index);
			for (int h_index = 0; h_index < (int)this->input_histories.size(); h_index++) {
				this->input_histories[h_index].erase(this->input_histories[h_index].begin() + i_index);
			}
		}
	}
}

CommitExperimentHistory::CommitExperimentHistory(CommitExperiment* experiment) {
	this->experiment = experiment;

	this->impact = 0.0;

	this->branch_experiment_history = NULL;
}

CommitExperimentHistory::~CommitExperimentHistory() {
	if (this->branch_experiment_history != NULL) {
		delete this->branch_experiment_history;
	}
}
