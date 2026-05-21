#include "experiment.h"

#include "network.h"
#include "world_model_helpers.h"

using namespace std;

void Experiment::experiment_activate(ExperimentRun& run) {
	ExperimentHistory* history;
	map<Experiment*, ExperimentHistory*>::iterator it =
		run.experiment_histories.find(this);
	if (it == run.experiment_histories.end()) {
		history = new ExperimentHistory(this);
		run.experiment_histories[this] = history;
	} else {
		history = it->second;
	}

	vector<double> inputs(this->input_indexes.size());
	for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
		inputs[i_index] = run.state[this->input_indexes[i_index]];
	}
	this->original_network->activate(inputs);
	this->branch_network->activate(inputs);

	if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
		history->hit_branch = true;

		if (history->is_on) {
			ExperimentState* new_experiment_state = new ExperimentState(this);
			new_experiment_state->step_index = 0;
			run.experiment_context = new_experiment_state;
		}
	}
}

void Experiment::experiment_step(vector<double>& obs,
								 int& action,
								 bool& is_next,
								 ExperimentRun& run) {
	if (run.experiment_context->step_index >= (int)this->actions.size()) {
		run.node_context = this->exit_next_node;

		delete run.experiment_context;
		run.experiment_context = NULL;
	} else {
		run.action_histories.push_back(this->actions[run.experiment_context->step_index]);

		action_helper(this->actions[run.experiment_context->step_index],
					  run.state,
					  run.wrapper);

		action = this->actions[run.experiment_context->step_index];
		is_next = true;

		run.experiment_context->step_index++;
	}
}

void Experiment::predict_activate(PredictRun& run) {
	ExperimentHistory* history;
	map<Experiment*, ExperimentHistory*>::iterator it =
		run.experiment_histories.find(this);
	if (it == run.experiment_histories.end()) {
		history = new ExperimentHistory(this);
		run.experiment_histories[this] = history;
	} else {
		history = it->second;
	}

	if (history->is_on) {
		vector<double> inputs(this->input_indexes.size());
		for (int i_index = 0; i_index < (int)this->input_indexes.size(); i_index++) {
			inputs[i_index] = run.state[this->input_indexes[i_index]];
		}
		this->original_network->activate(inputs);
		this->branch_network->activate(inputs);

		if (this->branch_network->output->acti_vals[0] >= this->original_network->output->acti_vals[0]) {
			ExperimentState* new_experiment_state = new ExperimentState(this);
			new_experiment_state->step_index = 0;
			run.experiment_context = new_experiment_state;
		}
	}
}

void Experiment::predict_step(PredictRun& run) {
	if (run.experiment_context->step_index >= (int)this->actions.size()) {
		run.node_context = this->exit_next_node;

		delete run.experiment_context;
		run.experiment_context = NULL;
	} else {
		action_helper(this->actions[run.experiment_context->step_index],
					  run.state,
					  run.wrapper);

		run.experiment_context->step_index++;
	}
}
