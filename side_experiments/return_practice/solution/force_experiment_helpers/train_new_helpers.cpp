#include "force_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "experiment_run.h"
#include "globals.h"
#include "network.h"
#include "predict_wrapper.h"
#include "solution.h"
#include "solution_helpers.h"
#include "state_network.h"
#include "start_node.h"
#include "world_model.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NEW_NUM_SAMPLES = 10;
#else
const int TRAIN_NEW_NUM_SAMPLES = 200;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_NUM_ITERS = 200;
#else
const int TRAIN_NUM_ITERS = 1000000;
#endif /* MDEBUG */

void ForceExperiment::train_new_experiment_activate(ExperimentRun* run) {
	ForceExperimentHistory* history = run->force_experiment_histories[this];

	this->original_network->activate(run->state);
	history->existing_predicted = this->original_network->output->acti_vals[0];

	history->branch_obs = run->obs_histories;
	history->branch_actions = run->action_histories;

	ForceExperimentState* new_experiment_state = new ForceExperimentState(this);
	new_experiment_state->step_index = 0;
	run->experiment_context = new_experiment_state;
}

void ForceExperiment::train_new_experiment_step(int& action,
												bool& is_next,
												ExperimentRun* run) {
	ForceExperimentState* state = (ForceExperimentState*)run->experiment_context;
	if (state->step_index >= (int)this->best_actions.size()) {
		run->node_context = this->exit_next_node;

		delete run->experiment_context;
		run->experiment_context = NULL;
	} else {
		run->action_histories.push_back(this->best_actions[state->step_index]);

		action_helper(this->best_actions[state->step_index],
					  run->state,
					  run->wrapper);

		action = this->best_actions[state->step_index];
		is_next = true;

		state->step_index++;
	}
}

void train_helper(vector<vector<double>>& obs,
				  vector<int>& actions,
				  double target_val,
				  StateNetwork* obs_network,
				  StateNetwork* action_network,
				  StateNetwork* temp_final_network,
				  int& epoch_iter,
				  double& average_max_update,
				  Wrapper* wrapper) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> state(world_model->num_states, 0.0);
	vector<double> force_state(FORCE_EXPERIMENT_NUM_NEW_STATE, 0.0);

	vector<StateNetworkHistory*> force_obs_network_histories;
	vector<StateNetworkHistory*> force_action_network_histories;

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), force_state.begin(), force_state.end());
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				obs_network->activate(inputs,
									  network_history);
				force_obs_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < FORCE_EXPERIMENT_NUM_NEW_STATE; o_index++) {
					force_state[o_index] += obs_network->output->acti_vals[o_index];
				}
			}
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];

			vector<double> partial_inputs;
			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
				if (action == a_index) {
					partial_inputs.push_back(1.0);
				} else {
					partial_inputs.push_back(0.0);
				}
			}

			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				world_model->action_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), force_state.begin(), force_state.end());
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				StateNetworkHistory* network_history = new StateNetworkHistory();
				action_network->activate(inputs,
										 network_history);
				force_action_network_histories.push_back(network_history);
				for (int o_index = 0; o_index < FORCE_EXPERIMENT_NUM_NEW_STATE; o_index++) {
					force_state[o_index] += action_network->output->acti_vals[o_index];
				}
			}
		}
	}

	vector<double> final_inputs;
	final_inputs.insert(final_inputs.end(), state.begin(), state.end());
	final_inputs.insert(final_inputs.end(), force_state.begin(), force_state.end());
	temp_final_network->activate(final_inputs);
	double predicted = temp_final_network->output->acti_vals[0];

	vector<double> final_errors{target_val - predicted};

	vector<double> force_state_errors(FORCE_EXPERIMENT_NUM_NEW_STATE, 0.0);

	temp_final_network->backprop(final_errors);
	for (int i_index = 0; i_index < FORCE_EXPERIMENT_NUM_NEW_STATE; i_index++) {
		force_state_errors[i_index] += temp_final_network->input->errors[world_model->num_states + i_index];
		temp_final_network->input->errors[world_model->num_states + i_index] = 0.0;
	}

	for (int step_index = (int)obs.size()-1; step_index >= 0; step_index--) {
		if (step_index < (int)actions.size()) {
			action_network->backprop(force_state_errors,
									 force_action_network_histories[step_index]);
			delete force_action_network_histories[step_index];
			for (int i_index = 0; i_index < FORCE_EXPERIMENT_NUM_NEW_STATE; i_index++) {
				force_state_errors[i_index] += action_network->input->errors[world_model->num_states + i_index];
				action_network->input->errors[world_model->num_states + i_index] = 0.0;
			}
		}

		{
			obs_network->backprop(force_state_errors,
								  force_obs_network_histories[step_index]);
			delete force_obs_network_histories[step_index];
			for (int i_index = 0; i_index < FORCE_EXPERIMENT_NUM_NEW_STATE; i_index++) {
				force_state_errors[i_index] += obs_network->input->errors[world_model->num_states + i_index];
				obs_network->input->errors[world_model->num_states + i_index] = 0.0;
			}
		}
	}

	epoch_iter++;
	if (epoch_iter >= NETWORK_EPOCH_SIZE) {
		double max_update = 0.0;
		obs_network->get_max_update(max_update);
		action_network->get_max_update(max_update);
		temp_final_network->get_max_update(max_update);

		average_max_update = 0.999*average_max_update + 0.001*max_update;

		if (max_update > 0.0) {
			double learning_rate = (0.3*NETWORK_TARGET_MAX_UPDATE) / average_max_update;
			if (learning_rate*max_update > NETWORK_TARGET_MAX_UPDATE) {
				learning_rate = NETWORK_TARGET_MAX_UPDATE/max_update;
			}

			obs_network->update_weights(learning_rate);
			action_network->update_weights(learning_rate);
			temp_final_network->update_weights(learning_rate);
		}

		epoch_iter = 0;
	}
}

void calc_branch_state_helper(vector<vector<double>>& obs,
							  vector<int>& actions,
							  StateNetwork* obs_network,
							  StateNetwork* action_network,
							  Wrapper* wrapper,
							  vector<double>& branch_state) {
	WorldModel* world_model = wrapper->world_model;

	vector<double> state(world_model->num_states, 0.0);
	vector<double> force_state(FORCE_EXPERIMENT_NUM_NEW_STATE, 0.0);

	for (int step_index = 0; step_index < (int)obs.size(); step_index++) {
		{
			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->obs_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				world_model->obs_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->obs_networks[n_index]->output->acti_vals[o_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), force_state.begin(), force_state.end());
				inputs.insert(inputs.end(), obs[step_index].begin(), obs[step_index].end());
				obs_network->activate(inputs);
				for (int o_index = 0; o_index < FORCE_EXPERIMENT_NUM_NEW_STATE; o_index++) {
					force_state[o_index] += obs_network->output->acti_vals[o_index];
				}
			}
		}

		if (step_index < (int)actions.size()) {
			int action = actions[step_index];

			vector<double> partial_inputs;
			for (int a_index = 0; a_index < wrapper->num_actions; a_index++) {
				if (action == a_index) {
					partial_inputs.push_back(1.0);
				} else {
					partial_inputs.push_back(0.0);
				}
			}

			vector<double> start_state = state;

			for (int n_index = 0; n_index < (int)world_model->action_networks.size(); n_index++) {
				vector<double> inputs;
				for (int i_index = 0; i_index < (int)world_model->network_inputs[n_index].size(); i_index++) {
					inputs.push_back(start_state[world_model->network_inputs[n_index][i_index]]);
				}
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				world_model->action_networks[n_index]->activate(inputs);
				for (int o_index = 0; o_index < (int)world_model->network_outputs[n_index].size(); o_index++) {
					state[world_model->network_outputs[n_index][o_index]] +=
						world_model->action_networks[n_index]->output->acti_vals[o_index];
				}
			}

			{
				vector<double> inputs;
				inputs.insert(inputs.end(), start_state.begin(), start_state.end());
				inputs.insert(inputs.end(), force_state.begin(), force_state.end());
				inputs.insert(inputs.end(), partial_inputs.begin(), partial_inputs.end());
				action_network->activate(inputs);
				for (int o_index = 0; o_index < FORCE_EXPERIMENT_NUM_NEW_STATE; o_index++) {
					force_state[o_index] += action_network->output->acti_vals[o_index];
				}
			}
		}
	}

	branch_state.insert(branch_state.end(), state.begin(), state.end());
	branch_state.insert(branch_state.end(), force_state.begin(), force_state.end());
}

void ForceExperiment::train_new_backprop(double target_val,
										 ExperimentRun* run,
										 ForceExperimentHistory* history,
										 Wrapper* wrapper) {
	this->existing_predicted.push_back(history->existing_predicted);
	this->new_branch_obs.push_back(history->branch_obs);
	this->new_branch_actions.push_back(history->branch_actions);
	this->new_full_obs.push_back(run->obs_histories);
	this->new_full_actions.push_back(run->action_histories);
	this->new_target_vals.push_back(target_val);

	if (this->new_branch_obs.size() >= TRAIN_NEW_NUM_SAMPLES) {
		// temp
		cout << "train_new" << endl;

		uniform_int_distribution<int> sample_distribution(0, this->new_branch_obs.size()-1);

		StateNetwork* obs_network = new StateNetwork(wrapper->world_model->num_states + FORCE_EXPERIMENT_NUM_NEW_STATE + wrapper->num_obs, FORCE_EXPERIMENT_NUM_NEW_STATE);
		StateNetwork* action_network = new StateNetwork(wrapper->world_model->num_states + FORCE_EXPERIMENT_NUM_NEW_STATE + wrapper->num_actions, FORCE_EXPERIMENT_NUM_NEW_STATE);
		StateNetwork* temp_final_network = new StateNetwork(wrapper->world_model->final_network);
		temp_final_network->add_inputs(FORCE_EXPERIMENT_NUM_NEW_STATE);
		temp_final_network->resize(wrapper->world_model->num_states + FORCE_EXPERIMENT_NUM_NEW_STATE);
		int epoch_iter = 0;
		double average_max_update = 0.0;

		for (int iter_index = 0; iter_index < TRAIN_NUM_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			train_helper(this->new_full_obs[index],
						 this->new_full_actions[index],
						 this->new_target_vals[index],
						 obs_network,
						 action_network,
						 temp_final_network,
						 epoch_iter,
						 average_max_update,
						 wrapper);

			// temp
			if (iter_index % 100000 == 0) {
				cout << iter_index << endl;
			}
		}

		delete temp_final_network;

		vector<vector<double>> train_new_state(this->new_branch_obs.size());
		for (int h_index = 0; h_index < (int)this->new_branch_obs.size(); h_index++) {
			vector<double> curr_branch_state;
			calc_branch_state_helper(this->new_branch_obs[h_index],
									 this->new_branch_actions[h_index],
									 obs_network,
									 action_network,
									 wrapper,
									 curr_branch_state);
			train_new_state[h_index] = curr_branch_state;
		}

		Network* branch_network = new Network(wrapper->world_model->num_states + FORCE_EXPERIMENT_NUM_NEW_STATE);
		double branch_hidden_1_average_max_update = 0.0;
		double branch_hidden_2_average_max_update = 0.0;
		double branch_hidden_3_average_max_update = 0.0;
		double branch_output_average_max_update = 0.0;
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = sample_distribution(generator);
			branch_network->activate(train_new_state[index]);
			double error = this->new_target_vals[index] - branch_network->output->acti_vals[0];
			branch_network->init_backprop(error,
										  branch_hidden_1_average_max_update,
										  branch_hidden_2_average_max_update,
										  branch_hidden_3_average_max_update,
										  branch_output_average_max_update);
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->new_branch_obs.size(); h_index++) {
			branch_network->activate(train_new_state[h_index]);
			if (branch_network->output->acti_vals[0] > this->existing_predicted[h_index]) {
				sum_vals += branch_network->output->acti_vals[0] - this->existing_predicted[h_index];
			}
		}
		double predicted_local_improvement = sum_vals / (double)this->new_branch_obs.size();

		double average_instances_per_run;
		switch (node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)node_context;
				average_instances_per_run = start_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)node_context;
				average_instances_per_run = action_node->average_instances_per_run;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)node_context;
				if (is_branch) {
					average_instances_per_run = branch_node->branch_average_instances_per_run;
				} else {
					average_instances_per_run = branch_node->original_average_instances_per_run;
				}
			}
			break;
		}

		double predicted_global_improvement = average_instances_per_run * predicted_local_improvement;

		// temp
		cout << "force_experiment" << endl;
		cout << "predicted_local_improvement: " << predicted_local_improvement << endl;
		cout << "predicted_global_improvement: " << predicted_global_improvement << endl;

		bool is_success = false;
		if (predicted_local_improvement > 0.0) {
			if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
						it != wrapper->solution->train_new_last_scores.end(); it++) {
					if (predicted_global_improvement >= *it) {
						num_better_than++;
					}
				}

				double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->train_new_last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->train_new_last_scores.size() >= NUM_LAST_TRACK) {
					wrapper->solution->train_new_last_scores.pop_front();
				}
				wrapper->solution->train_new_last_scores.push_back(predicted_global_improvement);
			} else {
				wrapper->solution->train_new_last_scores.push_back(predicted_global_improvement);
			}
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%3 != 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			// temp
			for (int i_index = 0; i_index < 10; i_index++) {
				cout << "start " << i_index << endl;
				view_world_model_helper(wrapper);
			}

			vector<int> new_network_inputs;
			for (int i_index = 0; i_index < wrapper->world_model->num_states + FORCE_EXPERIMENT_NUM_NEW_STATE; i_index++) {
				new_network_inputs.push_back(i_index);
			}
			wrapper->world_model->network_inputs.push_back(new_network_inputs);
			vector<int> new_network_outputs;
			for (int o_index = 0; o_index < FORCE_EXPERIMENT_NUM_NEW_STATE; o_index++) {
				new_network_outputs.push_back(wrapper->world_model->num_states + o_index);
			}
			wrapper->world_model->network_outputs.push_back(new_network_outputs);
			for (int i_index = 0; i_index < (int)obs_network->input->errors.size(); i_index++) {
				obs_network->input->errors[i_index] = 0.0;
			}
			wrapper->world_model->obs_networks.push_back(obs_network);
			for (int i_index = 0; i_index < (int)action_network->input->errors.size(); i_index++) {
				action_network->input->errors[i_index] = 0.0;
			}
			wrapper->world_model->action_networks.push_back(action_network);

			wrapper->world_model->num_states += FORCE_EXPERIMENT_NUM_NEW_STATE;

			wrapper->world_model->final_network->add_inputs(FORCE_EXPERIMENT_NUM_NEW_STATE);
			wrapper->world_model->final_network->resize(wrapper->world_model->num_states);

			wrapper->world_model->predict->add_states(FORCE_EXPERIMENT_NUM_NEW_STATE);

			wrapper->world_model->candidate_predict->add_states(FORCE_EXPERIMENT_NUM_NEW_STATE);

			wrapper->solution->pad_new_state(FORCE_EXPERIMENT_NUM_NEW_STATE);

			original_network->add_inputs(FORCE_EXPERIMENT_NUM_NEW_STATE);

			for (int i_index = 0; i_index < (int)branch_network->input->errors.size(); i_index++) {
				branch_network->input->errors[i_index] = 0.0;
			}

			finalize_helper(this->node_context,
							this->is_branch,
							this->best_actions,
							this->exit_next_node,
							this->original_network,
							branch_network,
							wrapper);

			// temp
			for (int i_index = 0; i_index < 10; i_index++) {
				cout << "end " << i_index << endl;
				view_world_model_helper(wrapper);
			}
		} else {
			delete obs_network;
			delete action_network;
			delete branch_network;
		}

		wrapper->experiment_iter++;

		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				start_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				action_node->experiment = NULL;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					branch_node->branch_experiment = NULL;
				} else {
					branch_node->original_experiment = NULL;
				}
			}
			break;
		}
		delete this;
	}
}
