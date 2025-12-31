#include "candidate_experiment.h"

#include <algorithm>

#include "constants.h"
#include "globals.h"
#include "network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "tunnel.h"

using namespace std;

const int TRUE_WEIGHT = 4;
const double MIN_TRUE_RATIO = 0.25;

const double POSITIVE_SEED_RATIO = 0.05;
const double NEGATIVE_SEED_RATIO = 0.5;

const int MAX_REFINE_EPOCHS = 10;

void select_parent_tunnel_helper(Scope* scope_context,
								 Scope*& parent_tunnel_parent,
								 int& parent_tunnel_index) {
	uniform_int_distribution<int> stack_trace_distribution(0, scope_context->explore_stack_traces.size()-1);
	int stack_trace_index = stack_trace_distribution(generator);
	vector<pair<Scope*, int>> possible_parents;
	for (int l_index = 0; l_index < (int)scope_context->explore_stack_traces[stack_trace_index].size()-1; l_index++) {
		/**
		 * - don't include scope_context's tunnels for now
		 */
		Scope* scope = scope_context->explore_stack_traces[stack_trace_index][l_index]->scope;
		for (int t_index = 0; t_index < (int)scope->tunnels.size(); t_index++) {
			possible_parents.push_back({scope, t_index});
		}
	}
	bool is_true;
	if (TRUE_WEIGHT / (TRUE_WEIGHT + possible_parents.size()) < MIN_TRUE_RATIO) {
		uniform_int_distribution<int> distribution(0, 3);
		if (distribution(generator) == 0) {
			is_true = true;
		} else {
			is_true = false;
		}
	} else {
		uniform_int_distribution<int> distribution(0, TRUE_WEIGHT + possible_parents.size() - 1);
		if (distribution(generator) < TRUE_WEIGHT) {
			is_true = true;
		} else {
			is_true = false;
		}
	}
	if (is_true) {
		parent_tunnel_parent = NULL;
		parent_tunnel_index = -1;
	} else {
		uniform_int_distribution<int> parent_distribution(0, possible_parents.size() - 1);
		int index = parent_distribution(generator);
		parent_tunnel_parent = possible_parents[index].first;
		parent_tunnel_index = possible_parents[index].second;
	}
}

void gather_training_data_helper(Scope* scope_context,
								 vector<vector<double>>& obs_vals,
								 vector<double>& target_vals) {
	Scope* parent_tunnel_parent;
	int parent_tunnel_index;
	select_parent_tunnel_helper(scope_context,
								parent_tunnel_parent,
								parent_tunnel_index);

	for (int h_index = 0; h_index < (int)scope_context->explore_stack_traces.size(); h_index++) {
		if (parent_tunnel_parent == NULL) {
			obs_vals.push_back(scope_context->explore_stack_traces[h_index].back()->obs_history);
			target_vals.push_back(scope_context->explore_target_val_histories[h_index]);
		} else {
			for (int l_index = 0; l_index < (int)scope_context->explore_stack_traces[h_index].size(); l_index++) {
				ScopeHistory* scope_history = scope_context->explore_stack_traces[h_index][l_index];
				if (scope_history->scope == parent_tunnel_parent) {
					if (!scope_history->tunnel_is_init[parent_tunnel_index]) {
						scope_history->tunnel_is_init[parent_tunnel_index] = true;
						Tunnel* tunnel = scope_history->scope->tunnels[parent_tunnel_index];
						scope_history->tunnel_vals[parent_tunnel_index] = tunnel->get_signal(scope_history->obs_history);
					}
					obs_vals.push_back(scope_context->explore_stack_traces[h_index].back()->obs_history);
					target_vals.push_back(scope_history->tunnel_vals[parent_tunnel_index]);

					break;
				}
			}
		}
	}
}

Tunnel* create_obs_candidate(vector<vector<double>>& obs_vals,
							 vector<double>& target_vals) {
	geometric_distribution<int> num_obs_distribution(0.5);
	int num_obs;
	while (true) {
		num_obs = 1 + num_obs_distribution(generator);
		if (num_obs <= (int)obs_vals[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(obs_vals[0].size());
	for (int i_index = 0; i_index < (int)obs_vals[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> inputs(obs_vals.size());
	for (int h_index = 0; h_index < (int)obs_vals.size(); h_index++) {
		inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[h_index][o_index] = obs_vals[h_index][obs_indexes[o_index]];
		}
	}

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	uniform_int_distribution<int> sample_distribution(0, inputs.size()-1);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = sample_distribution(generator);

		signal_network->activate(inputs[index]);

		double error = target_vals[index] - signal_network->output->acti_vals[0];

		signal_network->backprop(error);
	}

	Tunnel* new_tunnel = new Tunnel(obs_indexes,
									false,
									NULL,
									signal_network);

	return new_tunnel;
}

Tunnel* create_pattern_candidate(vector<vector<double>>& obs_vals,
								 vector<double>& target_vals) {
	geometric_distribution<int> num_obs_distribution(0.3);
	int num_obs;
	while (true) {
		num_obs = 2 + num_obs_distribution(generator);
		if (num_obs <= (int)obs_vals[0].size()) {
			break;
		}
	}

	vector<int> remaining_indexes(obs_vals[0].size());
	for (int i_index = 0; i_index < (int)obs_vals[0].size(); i_index++) {
		remaining_indexes[i_index] = i_index;
	}

	vector<int> obs_indexes;
	for (int o_index = 0; o_index < num_obs; o_index++) {
		uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
		int index = distribution(generator);
		obs_indexes.push_back(remaining_indexes[index]);
		remaining_indexes.erase(remaining_indexes.begin() + index);
	}

	vector<vector<double>> inputs(obs_vals.size());
	for (int h_index = 0; h_index < (int)obs_vals.size(); h_index++) {
		inputs[h_index] = vector<double>(num_obs);
		for (int o_index = 0; o_index < num_obs; o_index++) {
			inputs[h_index][o_index] = obs_vals[h_index][obs_indexes[o_index]];
		}
	}

	uniform_int_distribution<int> sample_distribution(0, inputs.size()-1);

	/**
	 * - initial
	 */
	vector<double> selected_inputs = inputs[0];
	double selected_target_val = target_vals[0];
	uniform_int_distribution<int> select_distribution(0, 1);
	for (int h_index = 1; h_index < (int)inputs.size(); h_index++) {
		if (target_vals[h_index] >= selected_target_val) {
			if (select_distribution(generator) == 0) {
				selected_inputs = inputs[h_index];
				selected_target_val = target_vals[h_index];
			}
		}
	}

	Network* similarity_network = new Network(num_obs,
											  NETWORK_SIZE_SMALL);

	uniform_int_distribution<int> seed_distribution(0, 9);
	uniform_int_distribution<int> non_seed_positive_distribution(0, 9);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 40; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 40000; iter_index++) {
	#endif /* MDEBUG */
		if (seed_distribution(generator) == 0) {
			similarity_network->activate(selected_inputs);

			if (similarity_network->output->acti_vals[0] < 1.0) {
				double error = 1.0 - similarity_network->output->acti_vals[0];
				similarity_network->backprop(error);
			}
		} else {
			int index = sample_distribution(generator);

			similarity_network->activate(inputs[index]);

			if (non_seed_positive_distribution(generator) == 0) {
				if (similarity_network->output->acti_vals[0] < 1.0) {
					double error = 1.0 - similarity_network->output->acti_vals[0];
					similarity_network->backprop(error);
				}
			} else {
				if (similarity_network->output->acti_vals[0] > 0.0) {
					double error = 0.0 - similarity_network->output->acti_vals[0];
					similarity_network->backprop(error);
				}
			}
		}
	}

	/**
	 * - refine
	 */
	int num_positive = POSITIVE_SEED_RATIO * (double)inputs.size();
	vector<int> positive_seed_indexes(num_positive);
	int num_negative = NEGATIVE_SEED_RATIO * (double)inputs.size();
	vector<int> negative_seed_indexes(num_negative);
	{
		vector<pair<double,int>> similarity_vals;
		for (int h_index = 0; h_index < (int)inputs.size(); h_index++) {
			similarity_network->activate(inputs[h_index]);
			similarity_vals.push_back({similarity_network->output->acti_vals[0], h_index});
		}
		sort(similarity_vals.begin(), similarity_vals.end());

		for (int i_index = 0; i_index < num_positive; i_index++) {
			positive_seed_indexes[i_index] = similarity_vals[inputs.size()-1 - i_index].second;
		}

		for (int i_index = 0; i_index < num_negative; i_index++) {
			negative_seed_indexes[i_index] = similarity_vals[i_index].second;
		}
	}

	uniform_int_distribution<int> is_positive_distribution(0, 11);
	uniform_int_distribution<int> positive_distribution(0, num_positive-1);
	uniform_int_distribution<int> negative_distribution(0, num_negative-1);
	for (int epoch_iter = 0; epoch_iter < MAX_REFINE_EPOCHS; epoch_iter++) {
		#if defined(MDEBUG) && MDEBUG
		for (int iter_index = 0; iter_index < 30; iter_index++) {
		#else
		for (int iter_index = 0; iter_index < 30000; iter_index++) {
		#endif /* MDEBUG */
			if (is_positive_distribution(generator) == 0) {
				int index = positive_distribution(generator);

				similarity_network->activate(inputs[positive_seed_indexes[index]]);

				if (similarity_network->output->acti_vals[0] < 1.0) {
					double error = 1.0 - similarity_network->output->acti_vals[0];
					similarity_network->backprop(error);
				}
			} else {
				int index = negative_distribution(generator);

				similarity_network->activate(inputs[negative_seed_indexes[index]]);

				if (similarity_network->output->acti_vals[0] > 0.0) {
					double error = 0.0 - similarity_network->output->acti_vals[0];
					similarity_network->backprop(error);
				}
			}
		}

		vector<pair<double,int>> similarity_vals;
		for (int h_index = 0; h_index < (int)inputs.size(); h_index++) {
			similarity_network->activate(inputs[h_index]);
			similarity_vals.push_back({similarity_network->output->acti_vals[0], h_index});
		}
		sort(similarity_vals.begin(), similarity_vals.end());

		if (similarity_vals[num_negative-1].first <= 0.0
				&& similarity_vals[inputs.size() - num_positive].first >= 1.0) {
			break;
		}

		for (int i_index = 0; i_index < num_positive; i_index++) {
			positive_seed_indexes[i_index] = similarity_vals[inputs.size()-1 - i_index].second;
		}

		for (int i_index = 0; i_index < num_negative; i_index++) {
			negative_seed_indexes[i_index] = similarity_vals[i_index].second;
		}
	}

	vector<double> similarity_vals(inputs.size());
	for (int h_index = 0; h_index < (int)inputs.size(); h_index++) {
		similarity_network->activate(inputs[h_index]);
		similarity_vals[h_index] = similarity_network->output->acti_vals[0];
	}

	Network* signal_network = new Network(num_obs,
										  NETWORK_SIZE_SMALL);
	#if defined(MDEBUG) && MDEBUG
	for (int iter_index = 0; iter_index < 30; iter_index++) {
	#else
	for (int iter_index = 0; iter_index < 300000; iter_index++) {
	#endif /* MDEBUG */
		int index = sample_distribution(generator);

		double similarity = similarity_vals[index];

		if (similarity > 0.0) {
			if (similarity > 1.0) {
				similarity = 1.0;
			}

			signal_network->activate(inputs[index]);

			double error = target_vals[index] - signal_network->output->acti_vals[0];
			double adjusted = similarity * error;

			signal_network->backprop(adjusted);
		}
	}

	Tunnel* new_tunnel = new Tunnel(obs_indexes,
									true,
									similarity_network,
									signal_network);

	return new_tunnel;
}

void CandidateExperiment::gather_tunnel_data_helper(ScopeHistory* scope_history,
													double& sum_vals) {
	Scope* scope = scope_history->scope;

	if (scope == this->scope_context) {
		double signal = this->candidate->get_signal(scope_history->obs_history);
		sum_vals += signal;
	} else {
		bool is_child = false;
		for (int c_index = 0; c_index < (int)scope->child_scopes.size(); c_index++) {
			if (scope->child_scopes[c_index] == this->scope_context) {
				is_child = true;
				break;
			}
		}
		if (is_child) {
			for (map<int, AbstractNodeHistory*>::iterator it = scope_history->node_histories.begin();
					it != scope_history->node_histories.end(); it++) {
				if (it->second->node->type == NODE_TYPE_SCOPE) {
					ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)it->second;
					gather_tunnel_data_helper(scope_node_history->scope_history,
											  sum_vals);
				}
			}
		}
	}
}

CandidateExperiment::CandidateExperiment(Scope* scope_context,
										 AbstractNode* node_context,
										 bool is_branch) {
	this->type = EXPERIMENT_TYPE_CANDIDATE;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;

	this->node_context->experiment = this;

	vector<vector<double>> obs_vals;
	vector<double> target_vals;
	gather_training_data_helper(this->scope_context,
								obs_vals,
								target_vals);

	uniform_int_distribution<int> pattern_distribution(0, 1);
	if (pattern_distribution(generator) == 0) {
		this->candidate = create_pattern_candidate(obs_vals,
												   target_vals);
	} else {
		this->candidate = create_obs_candidate(obs_vals,
											   target_vals);
	}

	this->existing_true_network = NULL;
	this->existing_signal_network = NULL;
	this->new_signal_network = NULL;

	this->curr_new_scope = NULL;
	this->best_new_scope = NULL;

	this->sum_num_instances = 0;

	this->sum_true = 0.0;
	this->hit_count = 0;

	this->state = CANDIDATE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->result = EXPERIMENT_RESULT_NA;
}

CandidateExperiment::~CandidateExperiment() {
	if (this->candidate != NULL) {
		delete this->candidate;
	}

	if (this->existing_true_network != NULL) {
		delete this->existing_true_network;
	}

	if (this->existing_signal_network != NULL) {
		delete this->existing_signal_network;
	}

	if (this->new_signal_network != NULL) {
		delete this->new_signal_network;
	}

	if (this->curr_new_scope != NULL) {
		delete this->curr_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->curr_new_nodes.size(); n_index++) {
		delete this->curr_new_nodes[n_index];
	}

	if (this->best_new_scope != NULL) {
		delete this->best_new_scope;
	}

	for (int n_index = 0; n_index < (int)this->best_new_nodes.size(); n_index++) {
		delete this->best_new_nodes[n_index];
	}

	for (int h_index = 0; h_index < (int)this->new_scope_histories.size(); h_index++) {
		delete this->new_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

CandidateExperimentHistory::CandidateExperimentHistory(CandidateExperiment* experiment) {
	this->experiment = experiment;

	this->is_hit = false;
}

CandidateExperimentState::CandidateExperimentState(CandidateExperiment* experiment) {
	this->experiment = experiment;
}
