#include "obs_experiment.h"

using namespace std;

/**
 * - practical limit
 *   - requires a huge increase to hidden size (i.e., runtime) to reliably find 5-way XORs
 */
const int OBS_LIMIT = 4;

const double MIN_IMPACT_SCALE = 0.3;

void ObsExperiment::hook(ObsExperimentHistory* obs_experiment_history) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];

			action_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			action_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			action_node->test_hook_histories.push_back(obs_experiment_history);
			action_node->test_hook_indexes.push_back(n_index);
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];

			scope_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			scope_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			scope_node->test_hook_obs_indexes.push_back(this->obs_indexes[n_index]);
			scope_node->test_hook_histories.push_back(obs_experiment_history);
			scope_node->test_hook_indexes.push_back(n_index);
		} else {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];

			branch_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			branch_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			branch_node->test_hook_histories.push_back(obs_experiment_history);
			branch_node->test_hook_indexes.push_back(n_index);
		}
	}
}

void ObsExperiment::unhook(ObsExperimentHistory* obs_experiment_history) {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];

			for (int h_index = 0; h_index < (int)action_node->test_hook_histories.size(); h_index++) {
				if (action_node->test_hook_histories[h_index] == obs_experiment_history) {
					action_node->test_hook_scope_contexts.erase(action_node->test_hook_scope_contexts.begin() + h_index);
					action_node->test_hook_node_contexts.erase(action_node->test_hook_node_contexts.begin() + h_index);
					action_node->test_hook_histories.erase(action_node->test_hook_histories.begin() + h_index);
					action_node->test_hook_indexes.erase(action_node->test_hook_indexes.begin() + h_index);
					break;
				}
			}
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];

			for (int h_index = 0; h_index < (int)scope_node->test_hook_histories.size(); h_index++) {
				if (scope_node->test_hook_histories[h_index] == obs_experiment_history) {
					scope_node->test_hook_scope_contexts.erase(scope_node->test_hook_scope_contexts.begin() + h_index);
					scope_node->test_hook_node_contexts.erase(scope_node->test_hook_node_contexts.begin() + h_index);
					scope_node->test_hook_obs_indexes.erase(scope_node->test_hook_obs_indexes.begin() + h_index);
					scope_node->test_hook_histories.erase(scope_node->test_hook_histories.begin() + h_index);
					scope_node->test_hook_indexes.erase(scope_node->test_hook_indexes.begin() + h_index);
					break;
				}
			}
		} else {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];

			for (int h_index = 0; h_index < (int)branch_node->test_hook_histories.size(); h_index++) {
				if (branch_node->test_hook_histories[h_index] == obs_experiment_history) {
					branch_node->test_hook_scope_contexts.erase(branch_node->test_hook_scope_contexts.begin() + h_index);
					branch_node->test_hook_node_contexts.erase(branch_node->test_hook_node_contexts.begin() + h_index);
					branch_node->test_hook_histories.erase(branch_node->test_hook_histories.begin() + h_index);
					branch_node->test_hook_indexes.erase(branch_node->test_hook_indexes.begin() + h_index);
					break;
				}
			}
		}
	}
}



void ObsExperiment::flat(ObsExperimentHistory* history,
						 double target_val,
						 double existing_predicted_score) {
	if (history->obs_indexes.size() > 0) {
		vector<double> flat_inputs(this->nodes.size(), 0.0);
		for (int o_index = 0; o_index < (int)history->obs_indexes.size(); o_index++) {
			flat_inputs[history->obs_indexes[o_index]] = history->obs_vals[o_index];
		}

		this->flat_network->activate(flat_inputs);

		double new_predicted_score = existing_predicted_score + this->flat_network->output->acti_vals[0];

		double predicted_score_error = target_val - new_predicted_score;

		this->flat_network->backprop(predicted_score_error);
	}
}

void ObsExperiment::trim() {
	vector<double> obs_impacts(this->nodes.size(), 0.0);
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		for (int h_index = 0; h_index < FLAT_NETWORK_HIDDEN_SIZE; h_index++) {
			obs_impacts[n_index] += abs(this->flat_network->hidden->weights[h_index][0][n_index]
				* this->flat_network->output->weights[0][0][h_index]);
		}
	}

	delete this->flat_network;
	this->flat_network = NULL;

	double max_impact = 0.0;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (obs_impacts[n_index] > max_impact) {
			max_impact = obs_impacts[n_index];
		}
	}

	vector<int> obs_indexes(this->nodes.size());
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		obs_indexes.push_back(n_index);
	}

	vector<AbstractNode*> new_nodes;
	vector<vector<int>> new_scope_contexts;
	vector<vector<int>> new_node_contexts;
	vector<int> new_obs_indexes;
	for (int l_index = 0; l_index < OBS_LIMIT; l_index++) {
		double highest_impact = 0.0;
		double highest_index = -1;
		for (int n_index = 0; n_index < (int)obs_impacts.size(); n_index++) {
			if (obs_impacts[n_index] > highest_impact) {
				highest_impact = obs_impacts[n_index];
				highest_index = n_index;
			}
		}

		if (highest_impact > MIN_IMPACT_SCALE*max_impact) {
			int original_index = obs_indexes[highest_index];

			new_nodes.push_back(this->nodes[original_index]);
			new_scope_contexts.push_back(this->scope_contexts[original_index]);
			new_node_contexts.push_back(this->node_contexts[original_index]);
			new_obs_indexes.push_back(this->obs_indexes[original_index]);

			obs_impacts.erase(obs_impacts.begin() + highest_index):
			obs_indexes.erase(obs_indexes.begin() + highest_index);
		} else {
			break;
		}
	}

	this->nodes = new_nodes;
	this->scope_contexts = new_scope_contexts;
	this->node_contexts = new_node_contexts;
	this->obs_indexes = new_obs_indexes;

	this->state_networks = vector<StateNetwork*>(this->nodes.size());
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		this->state_networks[n_index] = new StateNetwork();
	}
	this->resolved_variance = 0.0;

	this->state = OBS_EXPERIMENT_STATE_RNN;
	this->state_iter = 0;
}

void ObsExperiment::rnn(ObsExperimentHistory* history,
						double target_val,
						double existing_predicted_score) {
	if (history->obs_indexes.size() > 0) {
		uniform_real_distribution<double> starting_distribution(-1.0, 1.0);
		double state_val = starting_distribution(generator);

		StateNetwork* last_network = NULL;
		vector<double> ending_state_vals(history->obs_indexes.size());
		for (int o_index = 0; o_index < (int)history->obs_indexes.size(); o_index++) {
			int network_index = history->obs_indexes[o_index];

			if (last_network != NULL) {
				this->state_networks[network_index]->starting_mean = 0.9999*this->state_networks[network_index]->starting_mean + 0.0001*state_val;
				double curr_variance = (this->state_networks[network_index]->starting_mean - state_val) * (this->state_networks[network_index]->starting_mean - state_val);
				this->state_networks[network_index]->starting_variance = 0.9999*this->state_networks[network_index]->starting_variance + 0.0001*curr_variance;

				last_network->ending_mean = 0.999*last_network->ending_mean + 0.001*this->state_networks[network_index]->starting_mean;
				last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*this->state_networks[network_index]->starting_variance;

				this->state_networks[network_index]->preceding_networks.insert(last_network);
			}

			this->state_networks[network_index]->activate(history->obs_vals[network_index],
														  state_val);

			last_network = this->state_networks[network_index];
			ending_state_vals[o_index] = state_val;
		}

		double curr_variance = state_val*state_val;
		this->resolved_variance = 0.9999*this->resolved_variance + 0.0001*curr_variance;

		last_network->ending_mean = 0.999*last_network->ending_mean + 0.0;
		last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*this->resolved_variance;

		last_network->can_be_end = true;

		double new_predicted_score = existing_predicted_score + state_val;

		double curr_misguess = (target_val - new_predicted_score) * (target_val - new_predicted_score);
		this->new_average_misguess = 0.9999*this->new_average_misguess + 0.0001*curr_misguess;

		double state_error = target_val - new_predicted_score;

		for (int o_index = (int)history->obs_indexes.size() - 1; o_index >= 0; o_index--) {
			int network_index = history->obs_indexes[o_index];

			double curr_covariance = (ending_state_vals[o_index] - this->state_networks[network_index]->ending_mean) * state_val;
			this->state_networks[network_index]->covariance_with_end = 0.9999*this->state_networks[network_index]->covariance_with_end + 0.0001*curr_covariance;

			this->state_networks[network_index]->backprop(state_error);
		}
	}
}

void ObsExperiment::scope_eval(Scope* parent) {
	double misguess_improvement = parent->average_misguess - this->new_average_misguess;
	double existing_misguess_standard_deviation = sqrt(parent->misguess_variance);
	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)
	double improvement_t_score = misguess_improvement
		/ existing_misguess_standard_deviation / sqrt(20000);
	if (improvement_t_score > 2.326) {	// >99%
		State* new_state = new State();

		new_state->resolved_standard_deviation = sqrt(this->resolved_variance);

		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			this->state_networks[n_index]->starting_standard_deviation = sqrt(this->state_networks[n_index]->starting_variance);

			this->state_networks[n_index]->ending_standard_deviation = sqrt(this->state_networks[n_index]->ending_variance);

			this->state_networks[n_index]->correlation_to_end = this->state_networks[n_index]->covariance_with_end
				/ this->state_networks[n_index]->ending_standard_deviation / new_state->resolved_standard_deviation;
		}
		new_state->networks = this->state_networks;

		new_state->scale = new Scale(1.0);

		new_state->nodes = this->nodes;

		new_state->id = parent->num_states;
		parent->num_states++;

		parent[new_state->id] = new_state;

		for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
			if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->nodes[n_index];

				action_node->score_state_scope_contexts.push_back(this->scope_contexts[n_index]);
				action_node->score_state_node_contexts.push_back(this->node_contexts[n_index]);
				action_node->score_state_defs.push_back(new_state);
				action_node->score_state_network_indexes.push_back(n_index);
			} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];

				scope_node->score_state_scope_contexts.push_back(this->scope_contexts[n_index]);
				scope_node->score_state_node_contexts.push_back(this->node_contexts[n_index]);
				scope_node->score_state_obs_indexes.push_back(this->obs_indexes[n_index]);
				scope_node->score_state_defs.push_back(new_state);
				scope_node->score_state_network_indexes.push_back(n_index);
			} else {
				BranchNode* branch_node = (BranchNode*)this->nodes[n_index];

				branch_node->score_state_scope_contexts.push_back(this->scope_contexts[n_index]);
				branch_node->score_state_node_contexts.push_back(this->node_contexts[n_index]);
				branch_node->score_state_defs.push_back(new_state);
				action_nodbranch_node->score_state_network_indexes.push_back(n_index);
			}
		}
	} else {
		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			delete this->state_networks[n_index];
		}
	}
}

void ObsExperiment::branch_experiment_eval(BranchExperiment* branch_experiment) {
	double misguess_improvement = branch_experiment->average_misguess - this->new_average_misguess;
	double existing_misguess_standard_deviation = sqrt(branch_experiment->misguess_variance);
	// 0.0001 rolling average variance approx. equal to 20000 average variance (?)
	double improvement_t_score = misguess_improvement
		/ existing_misguess_standard_deviation / sqrt(20000);
	if (improvement_t_score > 2.326) {	// >99%
		State* new_state = new State();

		new_state->resolved_standard_deviation = sqrt(this->resolved_variance);

		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			this->state_networks[n_index]->starting_standard_deviation = sqrt(this->state_networks[n_index]->starting_variance);

			this->state_networks[n_index]->ending_standard_deviation = sqrt(this->state_networks[n_index]->ending_variance);

			this->state_networks[n_index]->correlation_to_end = this->state_networks[n_index]->covariance_with_end
				/ this->state_networks[n_index]->ending_standard_deviation / new_state->resolved_standard_deviation;
		}
		new_state->networks = this->state_networks;

		new_state->scale = new Scale(1.0);

		new_state->nodes = this->nodes;

		branch_experiment->new_score_states.push_back(new_state);
		branch_experiment->new_score_state_nodes.push_back(this->nodes);
		branch_experiment->new_score_state_scope_contexts.push_back(this->scope_contexts);
		branch_experiment->new_score_state_node_contexts.push_back(this->node_contexts);
		branch_experiment->new_score_state_obs_indexes.push_back(this->obs_indexes);
	} else {
		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			delete this->state_networks[n_index];
		}
	}
}
