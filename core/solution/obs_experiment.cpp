#include "obs_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_experiment.h"
#include "branch_node.h"
#include "constants.h"
#include "flat_network.h"
#include "globals.h"
#include "helpers.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int FLAT_ITERS = 500000;
const int RNN_ITERS = 500000;

/**
 * - practical limit
 *   - requires a huge increase to hidden size (i.e., runtime) to reliably find 5-way XORs
 */
const int OBS_LIMIT = 4;

const double MIN_IMPACT_SCALE = 0.3;

ObsExperiment::ObsExperiment() {
	this->resolved_variance = 1.0;

	this->existing_misguess = 0.0;
	this->new_misguess = 0.0;
}

ObsExperiment::~ObsExperiment() {
	for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
		delete this->state_networks[n_index];
	}
}

void ObsExperiment::hook() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			action_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			action_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			action_node->test_hook_indexes.push_back(n_index);
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			scope_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			scope_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			scope_node->test_hook_obs_indexes.push_back(this->obs_indexes[n_index]);
			scope_node->test_hook_indexes.push_back(n_index);
		} else {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			branch_node->test_hook_scope_contexts.push_back(this->scope_contexts[n_index]);
			branch_node->test_hook_node_contexts.push_back(this->node_contexts[n_index]);
			branch_node->test_hook_indexes.push_back(n_index);
		}
	}
}

void ObsExperiment::unhook() {
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (this->nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)this->nodes[n_index];
			action_node->test_hook_scope_contexts.clear();
			action_node->test_hook_node_contexts.clear();
			action_node->test_hook_indexes.clear();
		} else if (this->nodes[n_index]->type == NODE_TYPE_SCOPE) {
			ScopeNode* scope_node = (ScopeNode*)this->nodes[n_index];
			scope_node->test_hook_scope_contexts.clear();
			scope_node->test_hook_node_contexts.clear();
			scope_node->test_hook_obs_indexes.clear();
			scope_node->test_hook_indexes.clear();
		} else {
			BranchNode* branch_node = (BranchNode*)this->nodes[n_index];
			branch_node->test_hook_scope_contexts.clear();
			branch_node->test_hook_node_contexts.clear();
			branch_node->test_hook_indexes.clear();
		}
	}
}

void ObsExperiment::flat_vals_branch_experiment_helper(
		vector<int>& scope_context,
		vector<int>& node_context,
		BranchExperimentHistory* branch_experiment_history,
		int d_index,
		int stride_size,
		vector<double>& flat_vals) {
	BranchExperiment* branch_experiment = branch_experiment_history->experiment;

	for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
		// leave node_context.back() as -1

		if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = branch_experiment_history->action_histories[s_index];
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			action_node->flat_vals_back_activate(scope_context,
												 node_context,
												 d_index,
												 stride_size,
												 flat_vals,
												 action_node_history);
		} else {
			flat_vals_helper(scope_context,
							 node_context,
							 branch_experiment_history->sequence_histories[s_index]->scope_history,
							 d_index,
							 stride_size,
							 flat_vals);
		}
	}
}

void ObsExperiment::flat_vals_helper(vector<int>& scope_context,
									 vector<int>& node_context,
									 ScopeHistory* scope_history,
									 int d_index,
									 int stride_size,
									 vector<double>& flat_vals) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->flat_vals_back_activate(scope_context,
													 node_context,
													 d_index,
													 stride_size,
													 flat_vals,
													 action_node_history);

				if (action_node_history->branch_experiment_history != NULL) {
					flat_vals_branch_experiment_helper(scope_context,
													   node_context,
													   action_node_history->branch_experiment_history,
													   d_index,
													   stride_size,
													   flat_vals);
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				flat_vals_helper(scope_context,
								 node_context,
								 scope_node_history->inner_scope_history,
								 d_index,
								 stride_size,
								 flat_vals);

				node_context.back() = -1;

				scope_node->flat_vals_back_activate(scope_context,
													node_context,
													d_index,
													stride_size,
													flat_vals,
													scope_node_history);

				if (scope_node_history->branch_experiment_history != NULL) {
					flat_vals_branch_experiment_helper(scope_context,
													   node_context,
													   scope_node_history->branch_experiment_history,
													   d_index,
													   stride_size,
													   flat_vals);
				}
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				branch_node->flat_vals_back_activate(scope_context,
													 node_context,
													 d_index,
													 stride_size,
													 flat_vals,
													 branch_node_history);

				if (branch_node_history->branch_experiment_history != NULL) {
					flat_vals_branch_experiment_helper(scope_context,
													   node_context,
													   branch_node_history->branch_experiment_history,
													   d_index,
													   stride_size,
													   flat_vals);
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void ObsExperiment::flat(vector<double>& flat_vals,
						 vector<double>& diffs) {
	int stride_size = (int)this->nodes.size();
	FlatNetwork flat_network(stride_size);

	uniform_int_distribution<int> distribution(0, NUM_DATAPOINTS-1);
	for (int iter_index = 0; iter_index < FLAT_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		for (int i_index = 0; i_index < stride_size; i_index++) {
			flat_network.input->acti_vals[i_index] = flat_vals[rand_index*stride_size + i_index];
		}

		flat_network.activate();

		double error = diffs[rand_index] - flat_network.output->acti_vals[0];

		flat_network.backprop(error);
	}

	vector<double> obs_impacts(this->nodes.size(), 0.0);
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		for (int h_index = 0; h_index < FLAT_NETWORK_HIDDEN_SIZE; h_index++) {
			obs_impacts[n_index] += abs(flat_network.hidden->weights[h_index][0][n_index]
				* flat_network.output->weights[0][0][h_index]);
		}
	}

	double max_impact = 0.0;
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		if (obs_impacts[n_index] > max_impact) {
			max_impact = obs_impacts[n_index];
		}
	}

	vector<int> remaining_obs_indexes(this->nodes.size());
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		remaining_obs_indexes[n_index] = n_index;
	}

	vector<AbstractNode*> new_nodes;
	vector<vector<int>> new_scope_contexts;
	vector<vector<int>> new_node_contexts;
	vector<int> new_obs_indexes;
	for (int l_index = 0; l_index < OBS_LIMIT; l_index++) {
		if (obs_impacts.size() == 0) {
			break;
		}

		double highest_impact = 0.0;
		int highest_index = -1;
		for (int n_index = 0; n_index < (int)obs_impacts.size(); n_index++) {
			if (obs_impacts[n_index] > highest_impact) {
				highest_impact = obs_impacts[n_index];
				highest_index = n_index;
			}
		}

		if (highest_impact > MIN_IMPACT_SCALE*max_impact) {
			int original_index = remaining_obs_indexes[highest_index];

			new_nodes.push_back(this->nodes[original_index]);
			new_scope_contexts.push_back(this->scope_contexts[original_index]);
			new_node_contexts.push_back(this->node_contexts[original_index]);
			new_obs_indexes.push_back(this->obs_indexes[original_index]);

			obs_impacts.erase(obs_impacts.begin() + highest_index);
			remaining_obs_indexes.erase(remaining_obs_indexes.begin() + highest_index);
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
		this->state_networks[n_index] = new StateNetwork(n_index);
	}
}

void ObsExperiment::rnn_vals_branch_experiment_helper(
		vector<int>& scope_context,
		vector<int>& node_context,
		BranchExperimentHistory* branch_experiment_history,
		vector<int>& i_obs_indexes,
		vector<double>& i_obs_vals) {
	BranchExperiment* branch_experiment = branch_experiment_history->experiment;

	for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
		// leave node_context.back() as -1

		if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
			ActionNodeHistory* action_node_history = branch_experiment_history->action_histories[s_index];
			ActionNode* action_node = (ActionNode*)action_node_history->node;
			action_node->rnn_vals_back_activate(scope_context,
												node_context,
												i_obs_indexes,
												i_obs_vals,
												action_node_history);
		} else {
			rnn_vals_helper(scope_context,
							node_context,
							branch_experiment_history->sequence_histories[s_index]->scope_history,
							i_obs_indexes,
							i_obs_vals);
		}
	}
}

void ObsExperiment::rnn_vals_helper(vector<int>& scope_context,
									vector<int>& node_context,
									ScopeHistory* scope_history,
									vector<int>& i_obs_indexes,
									vector<double>& i_obs_vals) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->rnn_vals_back_activate(scope_context,
													node_context,
													i_obs_indexes,
													i_obs_vals,
													action_node_history);

				if (action_node_history->branch_experiment_history != NULL) {
					rnn_vals_branch_experiment_helper(scope_context,
													  node_context,
													  action_node_history->branch_experiment_history,
													  i_obs_indexes,
													  i_obs_vals);
				}
			} else if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_SCOPE) {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				rnn_vals_helper(scope_context,
								node_context,
								scope_node_history->inner_scope_history,
								i_obs_indexes,
								i_obs_vals);

				node_context.back() = -1;

				scope_node->rnn_vals_back_activate(scope_context,
												   node_context,
												   i_obs_indexes,
												   i_obs_vals,
												   scope_node_history);

				if (scope_node_history->branch_experiment_history != NULL) {
					rnn_vals_branch_experiment_helper(scope_context,
													  node_context,
													  scope_node_history->branch_experiment_history,
													  i_obs_indexes,
													  i_obs_vals);
				}
			} else {
				BranchNodeHistory* branch_node_history = (BranchNodeHistory*)scope_history->node_histories[i_index][h_index];
				BranchNode* branch_node = (BranchNode*)branch_node_history->node;
				branch_node->rnn_vals_back_activate(scope_context,
													node_context,
													i_obs_indexes,
													i_obs_vals,
													branch_node_history);

				if (branch_node_history->branch_experiment_history != NULL) {
					rnn_vals_branch_experiment_helper(scope_context,
													  node_context,
													  branch_node_history->branch_experiment_history,
													  i_obs_indexes,
													  i_obs_vals);
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void ObsExperiment::rnn(vector<double>& diffs) {
	uniform_real_distribution<double> starting_val_distribution(-1.0, 1.0);
	uniform_int_distribution<int> distribution(0, NUM_DATAPOINTS-1);
	for (int iter_index = 0; iter_index < FLAT_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		if (this->d_obs_indexes[rand_index].size() > 0) {
			double state_val = starting_val_distribution(generator);

			StateNetwork* last_network = NULL;
			for (int o_index = 0; o_index < (int)this->d_obs_indexes[rand_index].size(); o_index++) {
				int network_index = this->d_obs_indexes[rand_index][o_index];

				if (last_network != NULL) {
					this->state_networks[network_index]->starting_mean = 0.9999*this->state_networks[network_index]->starting_mean + 0.0001*state_val;
					double curr_variance = (this->state_networks[network_index]->starting_mean - state_val) * (this->state_networks[network_index]->starting_mean - state_val);
					this->state_networks[network_index]->starting_variance = 0.9999*this->state_networks[network_index]->starting_variance + 0.0001*curr_variance;

					last_network->ending_mean = 0.999*last_network->ending_mean + 0.001*this->state_networks[network_index]->starting_mean;
					last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*this->state_networks[network_index]->starting_variance;

					this->state_networks[network_index]->preceding_network_indexes.insert(last_network->index);
				}

				this->state_networks[network_index]->activate(this->d_obs_vals[rand_index][o_index],
															  state_val);

				last_network = this->state_networks[network_index];
			}

			double curr_variance = state_val*state_val;
			this->resolved_variance = 0.9999*this->resolved_variance + 0.0001*curr_variance;

			last_network->ending_mean = 0.999*last_network->ending_mean + 0.0;
			last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*this->resolved_variance;

			double error = diffs[rand_index] - state_val;

			for (int o_index = (int)this->d_obs_indexes[rand_index].size() - 1; o_index >= 0; o_index--) {
				int network_index = this->d_obs_indexes[rand_index][o_index];
				this->state_networks[network_index]->backprop(error);
			}
		}
	}
}

void ObsExperiment::evaluate(vector<double>& diffs) {
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		if (this->d_obs_indexes[d_index].size() > 0) {
			double state_val = 0.0;

			for (int o_index = 0; o_index < (int)this->d_obs_indexes[d_index].size(); o_index++) {
				int network_index = this->d_obs_indexes[d_index][o_index];
				this->state_networks[network_index]->activate(this->d_obs_vals[d_index][o_index],
															  state_val);
			}

			double curr_misguess = diffs[d_index] - state_val;
			this->new_misguess += curr_misguess*curr_misguess;
		} else {
			this->new_misguess += diffs[d_index]*diffs[d_index];
		}

		this->existing_misguess += diffs[d_index]*diffs[d_index];
	}
}

void ObsExperiment::experiment(list<ScopeHistory*>& scope_histories,
							   vector<double>& diffs) {
	hook();

	vector<double> flat_vals(NUM_DATAPOINTS * this->nodes.size(), 0.0);
	{
		int d_index = 0;
		for (list<ScopeHistory*>::iterator it = scope_histories.begin();
				it != scope_histories.end(); it++) {
			vector<int> scope_context;
			vector<int> node_context;
			flat_vals_helper(scope_context,
							 node_context,
							 *it,
							 d_index,
							 this->nodes.size(),
							 flat_vals);
			d_index++;
		}
	}

	unhook();

	flat(flat_vals,
		 diffs);

	hook();

	this->d_obs_indexes = vector<vector<int>>(NUM_DATAPOINTS);
	this->d_obs_vals = vector<vector<double>>(NUM_DATAPOINTS);
	{
		int d_index = 0;
		for (list<ScopeHistory*>::iterator it = scope_histories.begin();
				it != scope_histories.end(); it++) {
			vector<int> scope_context;
			vector<int> node_context;
			rnn_vals_helper(scope_context,
							node_context,
							*it,
							this->d_obs_indexes[d_index],
							this->d_obs_vals[d_index]);
			d_index++;
		}
	}

	unhook();

	rnn(diffs);

	evaluate(diffs);
}

bool ObsExperiment::scope_eval(Scope* parent) {
	double existing_average_misguess = this->existing_misguess / NUM_DATAPOINTS;
	double new_average_misguess = this->new_misguess / NUM_DATAPOINTS;

	double misguess_improvement = existing_average_misguess - new_average_misguess;
	double misguess_standard_deviation = sqrt(parent->misguess_variance);
	double improvement_t_score = misguess_improvement
		/ (misguess_standard_deviation / sqrt(NUM_DATAPOINTS));

	cout << "scope experiment nodes:";
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		cout << " " << this->nodes[n_index]->id;
	}
	cout << endl;
	cout << "existing_average_misguess: " << existing_average_misguess << endl;
	cout << "new_average_misguess: " << new_average_misguess << endl;
	cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
	cout << "improvement_t_score: " << improvement_t_score << endl;
	cout << "impact: " << sqrt(this->resolved_variance) << endl;
	cout << endl;

	if (improvement_t_score > 2.326) {	// >99%
		State* new_state = new State();

		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			this->state_networks[n_index]->starting_standard_deviation = sqrt(this->state_networks[n_index]->starting_variance);

			this->state_networks[n_index]->ending_standard_deviation = sqrt(this->state_networks[n_index]->ending_variance);

			this->state_networks[n_index]->parent_state = new_state;
		}
		new_state->networks = this->state_networks;
		this->state_networks.clear();

		new_state->id = solution->state_counter;
		solution->state_counter++;

		solution->states[new_state->id] = new_state;

		add_state(parent,
				  new_state,
				  sqrt(this->resolved_variance),
				  this->nodes,
				  this->scope_contexts,
				  this->node_contexts,
				  this->obs_indexes);

		// ofstream solution_save_file;
		// solution_save_file.open("saves/solution.txt");
		// solution->save(solution_save_file);
		// solution_save_file.close();

		return true;
	} else {
		return false;
	}
}

bool ObsExperiment::branch_experiment_eval(BranchExperiment* branch_experiment,
										   bool update_starting) {
	double existing_average_misguess = this->existing_misguess / NUM_DATAPOINTS;
	double new_average_misguess = this->new_misguess / NUM_DATAPOINTS;

	double misguess_improvement = existing_average_misguess - new_average_misguess;
	double existing_misguess_standard_deviation = sqrt(branch_experiment->new_misguess_variance);
	double improvement_t_score = misguess_improvement
		/ (existing_misguess_standard_deviation / sqrt(NUM_DATAPOINTS));

	cout << "experiment experiment nodes:";
	for (int n_index = 0; n_index < (int)this->nodes.size(); n_index++) {
		cout << " " << this->nodes[n_index]->id;
	}
	cout << endl;
	cout << "existing_average_misguess: " << existing_average_misguess << endl;
	cout << "new_average_misguess: " << new_average_misguess << endl;
	cout << "existing_misguess_standard_deviation: " << existing_misguess_standard_deviation << endl;
	cout << "improvement_t_score: " << improvement_t_score << endl;
	cout << "impact: " << sqrt(this->resolved_variance) << endl;

	bool result;
	if (improvement_t_score > 2.326) {	// >99%
		result = true;
		cout << "success" << endl;

		State* new_state = new State();

		for (int n_index = 0; n_index < (int)this->state_networks.size(); n_index++) {
			this->state_networks[n_index]->starting_standard_deviation = sqrt(this->state_networks[n_index]->starting_variance);

			this->state_networks[n_index]->ending_standard_deviation = sqrt(this->state_networks[n_index]->ending_variance);

			this->state_networks[n_index]->parent_state = new_state;
		}
		new_state->networks = this->state_networks;
		this->state_networks.clear();

		new_state->id = solution->state_counter;
		solution->state_counter++;

		if (update_starting) {
			branch_experiment->new_starting_experiment_state_weights.push_back(sqrt(this->resolved_variance));
		} else {
			branch_experiment->new_starting_experiment_state_weights.push_back(0.0);
		}

		branch_experiment->new_states.push_back(new_state);
		branch_experiment->new_state_nodes.push_back(this->nodes);
		branch_experiment->new_state_scope_contexts.push_back(this->scope_contexts);
		branch_experiment->new_state_node_contexts.push_back(this->node_contexts);
		branch_experiment->new_state_obs_indexes.push_back(this->obs_indexes);
		branch_experiment->new_state_weights.push_back(sqrt(this->resolved_variance));
	} else {
		result = false;
	}

	cout << endl;

	return result;
}

void ObsExperiment::update_diffs(vector<double>& diffs) {
	for (int d_index = 0; d_index < NUM_DATAPOINTS; d_index++) {
		if (this->d_obs_indexes[d_index].size() > 0) {
			double state_val = 0.0;

			for (int o_index = 0; o_index < (int)this->d_obs_indexes[d_index].size(); o_index++) {
				int network_index = this->d_obs_indexes[d_index][o_index];
				this->state_networks[network_index]->activate(this->d_obs_vals[d_index][o_index],
															  state_val);
			}

			diffs[d_index] -= state_val;
		}
	}
}
