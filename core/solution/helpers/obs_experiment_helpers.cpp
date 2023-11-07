#include "helpers.h"

using namespace std;

/**
 * - practical limit
 *   - increasing increases combinations checked by O(n^4)
 *     - but greatly increases noise, requiring an exponential(?) increase in hidden size (i.e., runtime) to solve
 */
const int NUM_INITIAL_OBS = 10;

const int FLAT_ITERS = 500000;
const int RNN_ITERS = 500000;

/**
 * - practical limit
 *   - requires a huge increase to hidden size (i.e., runtime) to reliably find 5-way XORs
 */
const int OBS_LIMIT = 4;

const double MIN_IMPACT_SCALE = 0.3;

void create_obs_experiment_helper(vector<int>& scope_context,
								  vector<int>& node_context,
								  vector<AbstractNode*>& possible_nodes,
								  vector<vector<int>>& possible_scope_contexts,
								  vector<vector<int>>& possible_node_contexts,
								  vector<int>& possible_obs_indexes,
								  ScopeHistory* scope_history);
void create_obs_experiment_experiment_helper(
		vector<int>& scope_context,
		vector<int>& node_context,
		vector<AbstractNode*>& possible_nodes,
		vector<vector<int>>& possible_scope_contexts,
		vector<vector<int>>& possible_node_contexts,
		vector<int>& possible_obs_indexes,
		AbstractExperimentHistory* experiment_history) {
	if (experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
		BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)experiment_history;
		BranchExperiment* branch_experiment = (BranchExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
			if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = branch_experiment->best_actions[s_index]->id;

				possible_nodes.push_back(branch_experiment->best_actions[s_index]);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);

				node_context.back() = -1;
			} else {
				node_context.back() = branch_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)branch_experiment_history->step_histories[s_index];

				create_obs_experiment_helper(scope_context,
											 node_context,
											 possible_nodes,
											 possible_scope_contexts,
											 possible_node_contexts,
											 possible_obs_indexes,
											 sequence_history->scope_history);

				node_context.back() = -1;
			}
		}
	} else {
		PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)experiment_history;
		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)pass_through_experiment_history->pre_step_histories.size(); s_index++) {
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = pass_through_experiment->best_actions[s_index]->id;

				possible_nodes.push_back(pass_through_experiment->best_actions[s_index]);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);

				node_context.back() = -1;
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->pre_step_histories[s_index];

				create_obs_experiment_helper(scope_context,
											 node_context,
											 possible_nodes,
											 possible_scope_contexts,
											 possible_node_contexts,
											 possible_obs_indexes,
											 sequence_history->scope_history);

				node_context.back() = -1;
			}
		}

		if (pass_through_experiment_history->branch_experiment_history != NULL) {
			create_obs_experiment_experiment_helper(scope_context,
													node_context,
													possible_nodes,
													possible_scope_contexts,
													possible_node_contexts,
													possible_obs_indexes,
													pass_through_experiment_history->branch_experiment_history);
		}

		for (int h_index = 0; h_index < (int)pass_through_experiment_history->post_step_histories.size(); h_index++) {
			int s_index = (int)pass_through_experiment->best_step_types.size() - (int)pass_through_experiment_history->post_step_histories.size() + h_index;
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				node_context.back() = pass_through_experiment->best_actions[s_index]->id;

				possible_nodes.push_back(pass_through_experiment->best_actions[s_index]);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);

				node_context.back() = -1;
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->post_step_histories[h_index];

				create_obs_experiment_helper(scope_context,
											 node_context,
											 possible_nodes,
											 possible_scope_contexts,
											 possible_node_contexts,
											 possible_obs_indexes,
											 sequence_history->scope_history);

				node_context.back() = -1;
			}
		}
	}
}

void create_obs_experiment_helper(vector<int>& scope_context,
								  vector<int>& node_context,
								  vector<AbstractNode*>& possible_nodes,
								  vector<vector<int>>& possible_scope_contexts,
								  vector<vector<int>>& possible_node_contexts,
								  vector<int>& possible_obs_indexes,
								  ScopeHistory* scope_history) {
	int scope_id = scope_history->scope->id;

	scope_context.push_back(scope_id);
	node_context.push_back(-1);

	for (int i_index = 0; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = 0; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			if (scope_history->node_histories[i_index][h_index]->node->type == NODE_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)scope_history->node_histories[i_index][h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;

				node_context.back() = action_node->id;

				possible_nodes.push_back(action_node);
				possible_scope_contexts.push_back(scope_context);
				possible_node_contexts.push_back(node_context);
				possible_obs_indexes.push_back(0);

				node_context.back() = -1;

				if (action_node_history->experiment_history != NULL) {
					create_obs_experiment_experiment_helper(
						scope_context,
						node_context,
						possible_nodes,
						possible_scope_contexts,
						possible_node_contexts,
						possible_obs_indexes,
						action_node_history->experiment_history);
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				uniform_int_distribution<int> distribution(0, 1);
				if (distribution(generator) == 0) {
					create_obs_experiment_helper(scope_context,
												 node_context,
												 possible_nodes,
												 possible_scope_contexts,
												 possible_node_contexts,
												 possible_obs_indexes,
												 scope_node_history->inner_scope_history);
				}

				if (!scope_node_history->is_early_exit) {
					for (map<int, StateStatus>::iterator it = scope_node_history->obs_snapshots.begin();
							it != scope_node_history->obs_snapshots.end(); it++) {
						possible_nodes.push_back(scope_node);
						possible_scope_contexts.push_back(scope_context);
						possible_node_contexts.push_back(node_context);
						possible_obs_indexes.push_back(it->first);
					}
				}

				node_context.back() = -1;

				if (scope_node_history->experiment_history != NULL) {
					create_obs_experiment_experiment_helper(
						scope_context,
						node_context,
						possible_nodes,
						possible_scope_contexts,
						possible_node_contexts,
						possible_obs_indexes,
						scope_node_history->experiment_history);
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void hook(vector<AbstractNode*>& nodes,
		  vector<vector<int>>& scope_contexts,
		  vector<vector<int>>& node_contexts,
		  vector<int>& obs_indexes) {
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)nodes[n_index];
			action_node->obs_experiment_scope_contexts = scope_contexts[n_index];
			action_node->obs_experiment_node_contexts = node_contexts[n_index];
			action_node->obs_experiment_index = n_index;
		} else {
			ScopeNode* scope_node = (ScopeNode*)nodes[n_index];
			scope_node->obs_experiment_scope_contexts.push_back(scope_contexts[n_index]);
			scope_node->obs_experiment_node_contexts.push_back(node_contexts[n_index]);
			scope_node->obs_experiment_obs_indexes.push_back(obs_indexes[n_index]);
			scope_node->obs_experiment_indexes.push_back(n_index);
		}
	}
}

void unhook(vector<AbstractNode*>& nodes) {
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (nodes[n_index]->type == NODE_TYPE_ACTION) {
			ActionNode* action_node = (ActionNode*)nodes[n_index];
			action_node->obs_experiment_scope_contexts.clear();
			action_node->obs_experiment_node_contexts.clear();
			action_node->obs_experiment_index = -1;
		} else {
			ScopeNode* scope_node = (ScopeNode*)nodes[n_index];
			scope_node->obs_experiment_scope_contexts.clear();
			scope_node->obs_experiment_node_contexts.clear();
			scope_node->obs_experiment_obs_indexes.clear();
			scope_node->obs_experiment_indexes.clear();
		}
	}
}

void flat_vals_experiment_helper(vector<int>& scope_context,
								 vector<int>& node_context,
								 AbstractExperimentHistory* experiment_history,
								 int d_index,
								 int stride_size,
								 vector<double>& flat_vals) {
	if (experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
		BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)experiment_history;
		BranchExperiment* branch_experiment = (BranchExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
			if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)branch_experiment_history->step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->flat_vals_back_activate(scope_context,
													 node_context,
													 d_index,
													 stride_size,
													 flat_vals,
													 action_node_history);
			} else {
				node_context.back() = branch_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)branch_experiment_history->step_histories[s_index];

				flat_vals_helper(scope_context,
								 node_context,
								 sequence_history->scope_history,
								 d_index,
								 stride_size,
								 flat_vals);

				node_context.back() = -1;
			}
		}
	} else {
		PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)experiment_history;
		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)pass_through_experiment_history->pre_step_histories.size(); s_index++) {
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->pre_step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->flat_vals_back_activate(scope_context,
													 node_context,
													 d_index,
													 stride_size,
													 flat_vals,
													 action_node_history);
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->pre_step_histories[s_index];

				flat_vals_helper(scope_context,
								 node_context,
								 sequence_history->scope_history,
								 d_index,
								 stride_size,
								 flat_vals);

				node_context.back() = -1;
			}
		}

		if (pass_through_experiment_history->branch_experiment_history != NULL) {
			flat_vals_experiment_helper(scope_context,
										node_context,
										pass_through_experiment_history->branch_experiment_history,
										d_index,
										stride_size,
										flat_vals);
		}

		for (int h_index = 0; h_index < (int)pass_through_experiment_history->post_step_histories.size(); h_index++) {
			int s_index = (int)pass_through_experiment->best_step_types.size() - (int)pass_through_experiment_history->post_step_histories.size() + h_index;
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->post_step_histories[h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->flat_vals_back_activate(scope_context,
													 node_context,
													 d_index,
													 stride_size,
													 flat_vals,
													 action_node_history);
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->post_step_histories[h_index];

				flat_vals_helper(scope_context,
								 node_context,
								 sequence_history->scope_history,
								 d_index,
								 stride_size,
								 flat_vals);

				node_context.back() = -1;
			}
		}
	}
}

void flat_vals_helper(vector<int>& scope_context,
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

				if (action_node_history->experiment_history != NULL) {
					flat_vals_experiment_helper(scope_context,
												node_context,
												action_node_history->experiment_history,
												d_index,
												stride_size,
												flat_vals);
				}
			} else {
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

				if (scope_node_history->experiment_history != NULL) {
					flat_vals_experiment_helper(scope_context,
												node_context,
												scope_node_history->experiment_history,
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

void rnn_vals_experiment_helper(vector<int>& scope_context,
								vector<int>& node_context,
								AbstractExperimentHistory* experiment_history,
								vector<int>& rnn_obs_experiment_indexes,
								vector<double>& rnn_vals) {
	if (experiment_history->experiment->type == EXPERIMENT_TYPE_BRANCH) {
		BranchExperimentInstanceHistory* branch_experiment_history = (BranchExperimentInstanceHistory*)experiment_history;
		BranchExperiment* branch_experiment = (BranchExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)branch_experiment->best_step_types.size(); s_index++) {
			if (branch_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)branch_experiment_history->step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->rnn_vals_back_activate(scope_context,
													node_context,
													rnn_obs_experiment_indexes,
													rnn_vals,
													action_node_history);
			} else {
				node_context.back() = branch_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)branch_experiment_history->step_histories[s_index];

				rnn_vals_helper(scope_context,
								node_context,
								sequence_history->scope_history,
								rnn_obs_experiment_indexes,
								rnn_vals);

				node_context.back() = -1;
			}
		}
	} else {
		PassThroughExperimentInstanceHistory* pass_through_experiment_history = (PassThroughExperimentInstanceHistory*)experiment_history;
		PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment_history->experiment;

		for (int s_index = 0; s_index < (int)pass_through_experiment_history->pre_step_histories.size(); s_index++) {
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->pre_step_histories[s_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->rnn_vals_back_activate(scope_context,
													node_context,
													rnn_obs_experiment_indexes,
													rnn_vals,
													action_node_history);
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->pre_step_histories[s_index];

				rnn_vals_helper(scope_context,
								node_context,
								sequence_history->scope_history,
								rnn_obs_experiment_indexes,
								rnn_vals);

				node_context.back() = -1;
			}
		}

		if (pass_through_experiment_history->branch_experiment_history != NULL) {
			rnn_vals_experiment_helper(scope_context,
									   node_context,
									   pass_through_experiment_history->branch_experiment_history,
									   rnn_obs_experiment_indexes,
									   rnn_vals);
		}

		for (int h_index = 0; h_index < (int)pass_through_experiment_history->post_step_histories.size(); h_index++) {
			int s_index = (int)pass_through_experiment->best_step_types.size() - (int)pass_through_experiment_history->post_step_histories.size() + h_index;
			if (pass_through_experiment->best_step_types[s_index] == STEP_TYPE_ACTION) {
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)pass_through_experiment_history->post_step_histories[h_index];
				ActionNode* action_node = (ActionNode*)action_node_history->node;
				action_node->rnn_vals_back_activate(scope_context,
													node_context,
													rnn_obs_experiment_indexes,
													rnn_vals,
													action_node_history);
			} else {
				node_context.back() = pass_through_experiment->best_sequences[s_index]->scope_node_placeholder->id;

				SequenceHistory* sequence_history = (SequenceHistory*)pass_through_experiment_history->post_step_histories[h_index];

				rnn_vals_helper(scope_context,
								node_context,
								sequence_history->scope_history,
								rnn_obs_experiment_indexes,
								rnn_vals);

				node_context.back() = -1;
			}
		}
	}
}

void rnn_vals_helper(vector<int>& scope_context,
					 vector<int>& node_context,
					 ScopeHistory* scope_history,
					 vector<int>& rnn_obs_experiment_indexes,
					 vector<double>& rnn_vals) {
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
													rnn_obs_experiment_indexes,
													rnn_vals,
													action_node_history);

				if (action_node_history->experiment_history != NULL) {
					rnn_vals_experiment_helper(scope_context,
											   node_context,
											   action_node_history->experiment_history,
											   rnn_obs_experiment_indexes,
											   rnn_vals);
				}
			} else {
				ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history->node_histories[i_index][h_index];
				ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;

				node_context.back() = scope_node->id;

				rnn_vals_helper(scope_context,
								node_context,
								scope_node_history->inner_scope_history,
								rnn_obs_experiment_indexes,
								rnn_vals);

				node_context.back() = -1;

				scope_node->rnn_vals_back_activate(scope_context,
												   node_context,
												   rnn_obs_experiment_indexes,
												   rnn_vals,
												   scope_node_history);

				if (scope_node_history->experiment_history != NULL) {
					rnn_vals_experiment_helper(scope_context,
											   node_context,
											   scope_node_history->experiment_history,
											   rnn_obs_experiment_indexes,
											   rnn_vals);
				}
			}
		}
	}

	scope_context.pop_back();
	node_context.pop_back();
}

void flat(vector<double>& flat_vals,
		  vector<double>& target_vals,
		  vector<AbstractNode*>& nodes,
		  vector<vector<int>>& scope_contexts,
		  vector<vector<int>>& node_contexts,
		  vector<int>& obs_indexes) {
	int stride_size = (int)nodes.size();
	FlatNetwork flat_network(stride_size);

	uniform_int_distribution<int> distribution(0, NUM_DATAPOINTS-1);
	for (int iter_index = 0; iter_index < FLAT_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		for (int i_index = 0; i_index < stride_size; i_index++) {
			flat_network.input->acti_vals[i_index] = flat_vals[rand_index*stride_size + i_index];
		}

		flat_network.activate();

		double error = target_vals[rand_index] - flat_network.output->acti_vals[0];

		flat_network.backprop(error);
	}

	vector<double> obs_impacts(nodes.size(), 0.0);
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		for (int h_index = 0; h_index < FLAT_NETWORK_HIDDEN_SIZE; h_index++) {
			obs_impacts[n_index] += abs(flat_network.hidden->weights[h_index][0][n_index]
				* flat_network.output->weights[0][0][h_index]);
		}
	}

	double max_impact = 0.0;
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		if (obs_impacts[n_index] > max_impact) {
			max_impact = obs_impacts[n_index];
		}
	}

	vector<int> remaining_obs_indexes(nodes.size());
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
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

			new_nodes.push_back(nodes[original_index]);
			new_scope_contexts.push_back(scope_contexts[original_index]);
			new_node_contexts.push_back(node_contexts[original_index]);
			new_obs_indexes.push_back(obs_indexes[original_index]);

			obs_impacts.erase(obs_impacts.begin() + highest_index);
			remaining_obs_indexes.erase(remaining_obs_indexes.begin() + highest_index);
		} else {
			break;
		}
	}

	nodes = new_nodes;
	scope_contexts = new_scope_contexts;
	node_contexts = new_node_contexts;
	obs_indexes = new_obs_indexes;
}

void rnn(vector<vector<int>>& rnn_obs_experiment_indexes,
		 vector<vector<double>>& rnn_vals,
		 vector<double>& target_vals,
		 vector<StateNetwork*>& state_networks,
		 double& resolved_variance) {
	int num_instances = (int)rnn_obs_experiment_indexes.size();

	uniform_int_distribution<int> distribution(0, num_instances-1);
	for (int iter_index = 0; iter_index < FLAT_ITERS; iter_index++) {
		int rand_index = distribution(generator);

		if (rnn_obs_experiment_indexes[rand_index].size() > 0) {
			double state_val = 0.0;

			StateNetwork* last_network = NULL;
			for (int o_index = 0; o_index < (int)rnn_obs_experiment_indexes[rand_index].size(); o_index++) {
				int network_index = rnn_obs_experiment_indexes[rand_index][o_index];

				if (last_network != NULL) {
					state_networks[network_index]->starting_mean = 0.9999*state_networks[network_index]->starting_mean + 0.0001*state_val;
					double curr_variance = (state_networks[network_index]->starting_mean - state_val) * (state_networks[network_index]->starting_mean - state_val);
					state_networks[network_index]->starting_variance = 0.9999*state_networks[network_index]->starting_variance + 0.0001*curr_variance;

					last_network->ending_mean = 0.999*last_network->ending_mean + 0.001*state_networks[network_index]->starting_mean;
					last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*state_networks[network_index]->starting_variance;

					state_networks[network_index]->preceding_network_indexes.insert(last_network->index);
				}

				state_networks[network_index]->activate(rnn_vals[rand_index][o_index],
														state_val);

				last_network = state_networks[network_index];
			}

			double curr_variance = state_val*state_val;
			resolved_variance = 0.9999*resolved_variance + 0.0001*curr_variance;

			last_network->ending_mean = 0.999*last_network->ending_mean + 0.0;
			last_network->ending_variance = 0.999*last_network->ending_variance + 0.001*resolved_variance;

			double error = diffs[rand_index] - state_val;

			for (int o_index = (int)rnn_obs_experiment_indexes[rand_index].size() - 1; o_index >= 0; o_index--) {
				int network_index = rnn_obs_experiment_indexes[rand_index][o_index];
				state_networks[network_index]->backprop(error);
			}
		}
	}
}

void evaluate(double& existing_average_misguess,
			  double& existing_misguess_variance,
			  double& new_misguess,
			  vector<vector<int>>& rnn_obs_experiment_indexes,
			  vector<vector<double>>& rnn_vals,
			  vector<double>& target_vals,
			  vector<StateNetwork*>& state_networks) {
	double sum_existing_misguess = 0.0;
	double sum_new_misguess = 0.0;
	for (int d_index = 0; d_index < (int)rnn_obs_experiment_indexes.size(); d_index++) {
		if (rnn_obs_experiment_indexes[d_index].size() > 0) {
			double state_val = 0.0;

			for (int o_index = 0; o_index < (int)rnn_obs_experiment_indexes[d_index].size(); o_index++) {
				int network_index = rnn_obs_experiment_indexes[d_index][o_index];
				state_networks[network_index]->activate(rnn_vals[d_index][o_index],
														state_val);
			}

			sum_new_misguess += (target_vals[d_index] - state_val) * (target_vals[d_index] - state_val);
		} else {
			sum_new_misguess += target_vals[d_index] * target_vals[d_index];
		}

		sum_existing_misguess += target_vals[d_index] * target_vals[d_index];
	}

	existing_average_misguess = sum_existing_misguess / rnn_obs_experiment_indexes.size();
	new_average_misguess = sum_new_misguess / rnn_obs_experiment_indexes.size();

	double sum_existing_misguess_variance = 0.0;
	for (int d_index = 0; d_index < (int)rnn_obs_experiment_indexes.size(); d_index++) {
		double curr_existing_misguess = target_vals[d_index] * target_vals[d_index];
		sum_existing_misguess_variance += (curr_existing_misguess - existing_average_misguess) * (curr_existing_misguess - existing_average_misguess);
	}

	existing_misguess_variance = sum_existing_misguess_variance / rnn_obs_experiment_indexes.size();
}

void existing_obs_experiment(AbstractExperiment* experiment,
							 Scope* parent_scope,
							 list<ScopeHistory*>& scope_histories,
							 vector<double>& target_vals) {
	vector<AbstractNode*> nodes;
	vector<vector<int>> scope_contexts;
	vector<vector<int>> node_contexts;
	vector<int> obs_indexes;
	{
		vector<AbstractNode*> possible_nodes;
		vector<vector<int>> possible_scope_contexts;
		vector<vector<int>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		vector<int> scope_context;
		vector<int> node_context;
		create_obs_experiment_helper(scope_context,
									 node_context,
									 possible_nodes,
									 possible_scope_contexts,
									 possible_node_contexts,
									 possible_obs_indexes,
									 scope_histories.back());
		/**
		 * - simply always use last ScopeHistory to create ObsExperiment
		 */

		int num_obs = min(NUM_INITIAL_OBS, (int)possible_nodes.size());
		for (int o_index = 0; o_index < num_obs; o_index++) {
			uniform_int_distribution<int> distribution(0, (int)possible_nodes.size()-1);
			int rand_obs = distribution(generator);

			nodes.push_back(possible_nodes[rand_obs]);
			scope_contexts.push_back(possible_scope_contexts[rand_obs]);
			node_contexts.push_back(possible_node_contexts[rand_obs]);
			obs_indexes.push_back(possible_obs_indexes[rand_obs]);

			possible_nodes.erase(possible_nodes.begin() + rand_obs);
			possible_scope_contexts.erase(possible_scope_contexts.begin() + rand_obs);
			possible_node_contexts.erase(possible_node_contexts.begin() + rand_obs);
			possible_obs_indexes.erase(possible_obs_indexes.begin() + rand_obs);
		}
	}

	int num_instances = (int)scope_histories.size();

	hook(nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	vector<double> flat_vals(num_instances * nodes.size(), 0.0);
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
							 nodes.size(),
							 flat_vals);
			d_index++;
		}
	}

	unhook(nodes);

	flat(flat_vals,
		 target_vals,
		 nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	hook(nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	vector<vector<int>> rnn_obs_experiment_indexes(num_instances);
	vector<vector<double>> rnn_vals(num_instances);
	{
		int d_index = 0;
		for (list<ScopeHistory*>::iterator it = scope_histories.begin();
				it != scope_histories.end(); it++) {
			vector<int> scope_context;
			vector<int> node_context;
			rnn_vals_helper(scope_context,
							node_context,
							*it,
							rnn_obs_experiment_indexes[d_index],
							rnn_vals[d_index]);
			d_index++;
		}
	}

	unhook(nodes);

	vector<StateNetwork*> state_networks(nodes.size());
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		state_networks[n_index] = new StateNetwork(n_index);
	}
	double resolved_variance = 1.0;
	rnn(rnn_obs_experiment_indexes,
		rnn_vals,
		target_vals,
		state_networks,
		resolved_variance);

	double existing_average_misguess;
	double existing_misguess_variance;
	double new_average_misguess;
	evaluate(existing_average_misguess,
			 existing_misguess_variance,
			 new_average_misguess,
			 rnn_obs_experiment_indexes,
			 rnn_vals,
			 target_vals,
			 state_networks);

	double misguess_improvement = existing_average_misguess - new_average_misguess;
	double misguess_standard_deviation = sqrt(existing_misguess_variance);
	double improvement_t_score = misguess_improvement
		/ (misguess_standard_deviation / sqrt(num_instances));

	cout << "existing experiment nodes:";
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		cout << " " << nodes[n_index]->id;
	}
	cout << endl;
	cout << "existing_average_misguess: " << existing_average_misguess << endl;
	cout << "new_average_misguess: " << new_average_misguess << endl;
	cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
	cout << "improvement_t_score: " << improvement_t_score << endl;
	cout << endl;

	if (improvement_t_score > 2.326) {	// >99%
		State* new_state = new State();

		for (int n_index = 0; n_index < (int)state_networks.size(); n_index++) {
			state_networks[n_index]->starting_standard_deviation = sqrt(state_networks[n_index]->starting_variance);

			state_networks[n_index]->ending_standard_deviation = sqrt(state_networks[n_index]->ending_variance);

			state_networks[n_index]->parent_state = new_state;
		}
		new_state->networks = state_networks;

		new_state->id = solution->state_counter;
		solution->state_counter++;

		parent_scope->temp_states.push_back(new_state);
		parent_scope->temp_state_nodes.push_back(nodes);
		parent_scope->temp_state_scope_contexts.push_back(scope_contexts);
		parent_scope->temp_state_node_contexts.push_back(node_contexts);
		parent_scope->temp_state_obs_indexes.push_back(obs_indexes);
		parent_scope->temp_state_new_local_indexes.push_back(-1);

		for (int n_index = 0; n_index < nodes.size(); n_index++) {
			if (nodes[n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)nodes[n_index];

				action_node->temp_state_scope_contexts.push_back(scope_contexts[n_index]);
				action_node->temp_state_node_contexts.push_back(node_contexts[n_index]);
				action_node->temp_state_defs.push_back(new_state);
				action_node->temp_state_network_indexes.push_back(n_index);
			} else {
				ScopeNode* scope_node = (ScopeNode*)nodes[n_index];

				scope_node->temp_state_scope_contexts.push_back(scope_contexts[n_index]);
				scope_node->temp_state_node_contexts.push_back(node_contexts[n_index]);
				scope_node->temp_state_obs_indexes.push_back(obs_indexes[n_index]);
				scope_node->temp_state_defs.push_back(new_state);
				scope_node->temp_state_network_indexes.push_back(n_index);
			}
		}

		if (experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperiment* branch_experiment = (BranchExperiment*)experiment;
			branch_experiment->existing_temp_state_weights[0][new_state] = sqrt(resolved_variance);
		} else {
			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment;
			pass_through_experiment->existing_temp_state_weights[0][new_state] = sqrt(resolved_variance);
		}
	} else {
		for (int n_index = 0; n_index < (int)state_networks.size(); n_index++) {
			delete state_networks[n_index];
		}
	}
}

void new_obs_experiment(AbstractExperiment* experiment,
						list<ScopeHistory*>& scope_histories,
						vector<double>& target_vals) {
	vector<AbstractNode*> nodes;
	vector<vector<int>> scope_contexts;
	vector<vector<int>> node_contexts;
	vector<int> obs_indexes;
	{
		vector<AbstractNode*> possible_nodes;
		vector<vector<int>> possible_scope_contexts;
		vector<vector<int>> possible_node_contexts;
		vector<int> possible_obs_indexes;

		vector<int> scope_context;
		vector<int> node_context;
		create_obs_experiment_helper(scope_context,
									 node_context,
									 possible_nodes,
									 possible_scope_contexts,
									 possible_node_contexts,
									 possible_obs_indexes,
									 scope_histories.back());
		/**
		 * - simply always use last ScopeHistory to create ObsExperiment
		 */

		int num_obs = min(NUM_INITIAL_OBS, (int)possible_nodes.size());
		for (int o_index = 0; o_index < num_obs; o_index++) {
			uniform_int_distribution<int> distribution(0, (int)possible_nodes.size()-1);
			int rand_obs = distribution(generator);

			nodes.push_back(possible_nodes[rand_obs]);
			scope_contexts.push_back(possible_scope_contexts[rand_obs]);
			node_contexts.push_back(possible_node_contexts[rand_obs]);
			obs_indexes.push_back(possible_obs_indexes[rand_obs]);

			possible_nodes.erase(possible_nodes.begin() + rand_obs);
			possible_scope_contexts.erase(possible_scope_contexts.begin() + rand_obs);
			possible_node_contexts.erase(possible_node_contexts.begin() + rand_obs);
			possible_obs_indexes.erase(possible_obs_indexes.begin() + rand_obs);
		}
	}

	int num_instances = (int)scope_histories.size();

	hook(nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	vector<double> flat_vals(num_instances * nodes.size(), 0.0);
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
							 nodes.size(),
							 flat_vals);
			d_index++;
		}
	}

	unhook(nodes);

	flat(flat_vals,
		 target_vals,
		 nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	hook(nodes,
		 scope_contexts,
		 node_contexts,
		 obs_indexes);

	vector<vector<int>> rnn_obs_experiment_indexes(num_instances);
	vector<vector<double>> rnn_vals(num_instances);
	{
		int d_index = 0;
		for (list<ScopeHistory*>::iterator it = scope_histories.begin();
				it != scope_histories.end(); it++) {
			vector<int> scope_context;
			vector<int> node_context;
			rnn_vals_helper(scope_context,
							node_context,
							*it,
							rnn_obs_experiment_indexes[d_index],
							rnn_vals[d_index]);
			d_index++;
		}
	}

	unhook(nodes);

	vector<StateNetwork*> state_networks(nodes.size());
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		state_networks[n_index] = new StateNetwork(n_index);
	}
	double resolved_variance = 1.0;
	rnn(rnn_obs_experiment_indexes,
		rnn_vals,
		target_vals,
		state_networks,
		resolved_variance);

	double existing_average_misguess;
	double existing_misguess_variance;
	double new_average_misguess;
	evaluate(existing_average_misguess,
			 existing_misguess_variance,
			 new_average_misguess,
			 rnn_obs_experiment_indexes,
			 rnn_vals,
			 target_vals,
			 state_networks);

	double misguess_improvement = existing_average_misguess - new_average_misguess;
	double misguess_standard_deviation = sqrt(existing_misguess_variance);
	double improvement_t_score = misguess_improvement
		/ (misguess_standard_deviation / sqrt(num_instances));

	cout << "existing experiment nodes:";
	for (int n_index = 0; n_index < (int)nodes.size(); n_index++) {
		cout << " " << nodes[n_index]->id;
	}
	cout << endl;
	cout << "existing_average_misguess: " << existing_average_misguess << endl;
	cout << "new_average_misguess: " << new_average_misguess << endl;
	cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
	cout << "improvement_t_score: " << improvement_t_score << endl;
	cout << endl;

	if (improvement_t_score > 2.326) {	// >99%
		State* new_state = new State();

		for (int n_index = 0; n_index < (int)state_networks.size(); n_index++) {
			state_networks[n_index]->starting_standard_deviation = sqrt(state_networks[n_index]->starting_variance);

			state_networks[n_index]->ending_standard_deviation = sqrt(state_networks[n_index]->ending_variance);

			state_networks[n_index]->parent_state = new_state;
		}
		new_state->networks = state_networks;

		new_state->id = solution->state_counter;
		solution->state_counter++;

		if (experiment->type == EXPERIMENT_TYPE_BRANCH) {
			BranchExperiment* branch_experiment = (BranchExperiment*)experiment;

			branch_experiment->new_states.push_back(new_state);
			branch_experiment->new_state_nodes.push_back(nodes);
			branch_experiment->new_state_scope_contexts.push_back(scope_contexts);
			branch_experiment->new_state_node_contexts.push_back(node_contexts);
			branch_experiment->new_state_obs_indexes.push_back(obs_indexes);

			branch_experiment->new_temp_state_weights[0][new_state] = sqrt(resolved_variance);
		} else {
			PassThroughExperiment* pass_through_experiment = (PassThroughExperiment*)experiment;

			pass_through_experiment->new_states.push_back(new_state);
			pass_through_experiment->new_state_nodes.push_back(nodes);
			pass_through_experiment->new_state_scope_contexts.push_back(scope_contexts);
			pass_through_experiment->new_state_node_contexts.push_back(node_contexts);
			pass_through_experiment->new_state_obs_indexes.push_back(obs_indexes);

			pass_through_experiment->new_temp_state_weights[0][new_state] = sqrt(resolved_variance);
		}
	} else {
		for (int n_index = 0; n_index < (int)state_networks.size(); n_index++) {
			delete state_networks[n_index];
		}
	}
}
