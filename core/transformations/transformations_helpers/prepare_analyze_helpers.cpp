#include "transformations_helpers.h"

using namespace std;

const int TRAIN_ITERS = 500000;

void prepare_analyze(Scope* parent_scope,
					 std::vector<AbstractScopeHistory*>& scope_histories) {
	map<AbstractNode*, int> action_node_to_index;
	for (map<int, AbstractNode*>::iterator it = parent_scope->nodes.begin();
			it != parent_scope->nodes.end(); it++) {
		if (it->second->type == NODE_TYPE_ACTION) {
			int index = (int)action_node_to_index.size();
			action_node_to_index[it->second] = index;
		}
	}
	int num_vals = (int)action_node_to_index.size() * problem_type->num_obs();

	int num_instances = (int)scope_histories.size();

	vector<double> sum_obs_means(num_vals, 0.0);
	vector<int> obs_counts(num_vals, 0);
	for (int d_index = 0; d_index < num_instances; d_index++) {
		for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_histories[d_index]->node_histories.begin();
				it != scope_histories[d_index]->node_histories.end(); it++) {
			if (it->first->type == NODE_TYPE_ACTION) {
				int node_index = action_node_to_index[it->first];
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
					int val_index = problem_type->num_obs() * node_index + o_index;
					sum_obs_means[val_index] += action_node_history->obs_snapshot[o_index];
					obs_counts[val_index]++;
				}
			}
		}
	}

	vector<double> obs_means(num_vals);
	for (int v_index = 0; v_index < num_vals; v_index++) {
		obs_means[v_index] = sum_obs_means[v_index] / obs_counts[v_index];
	}

	vector<double> sum_obs_variances(num_vals, 0.0);
	for (int d_index = 0; d_index < num_instances; d_index++) {
		for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_histories[d_index]->node_histories.begin();
				it != scope_histories[d_index]->node_histories.end(); it++) {
			if (it->first->type == NODE_TYPE_ACTION) {
				int index = action_node_to_index[it->first];
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
					int val_index = problem_type->num_obs() * node_index + o_index;
					sum_obs_variances[val_index] += (action_node_history->obs_snapshot[o_index] - obs_means[val_index]) * (action_node_history->obs_snapshot[o_index] - obs_means[val_index]);
				}
			}
		}
	}

	vector<double> obs_standard_deviations(num_vals);
	for (int v_index = 0; v_index < num_vals; v_index++) {
		obs_standard_deviations[v_index] = sqrt(
			sum_obs_variances[v_index] / obs_counts[v_index]);
	}

	AllToAllNetwork* network = new AllToAllNetwork(num_vals);

	vector<vector<double>> vals(num_instances);
	for (int d_index = 0; d_index < num_instances; d_index++) {
		vector<double> curr_vals(num_vals, 0.0);
		for (map<AbstractNode*, AbstractNodeHistory*>::iterator it = scope_histories[d_index]->node_histories.begin();
				it != scope_histories[d_index]->node_histories.end(); it++) {
			if (it->first->type == NODE_TYPE_ACTION) {
				int index = action_node_to_index[it->first];
				ActionNodeHistory* action_node_history = (ActionNodeHistory*)it->second;
				for (int o_index = 0; o_index < problem_type->num_obs(); o_index++) {
					int val_index = problem_type->num_obs() * node_index + o_index;
					double normalized_val = (action_node_history->obs_snapshot[o_index] - obs_means[val_index]) / obs_standard_deviations[val_index];
					curr_vals[val_index] = normalized_val;
				}
			}
		}
		vals[d_index] = curr_vals;
	}

	uniform_int_distribution<int> train_distribution(0, num_instances-1);
	uniform_int_distribution<int> is_output_distribution(0, 4);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = train_distribution(generator);

		vector<bool> is_output(num_vals);
		for (int v_index = 0; v_index < num_vals; v_index++) {
			is_output[v_index] = is_output_distribution(generator);
		}

		vector<double> inputs(num_vals);
		for (int v_index = 0; v_index < num_vals; v_index++) {
			if (is_output[v_index]) {
				inputs[v_index] = 0.0;
			} else {
				inputs[v_index] = vals[rand_index][v_index];
			}
		}

		network->activate(inputs);

		vector<double> errors(num_vals);
		for (int v_index = 0; v_index < num_vals; v_index++) {
			if (is_output[v_index]) {
				errors[v_index] = vals[rand_index][v_index] - network->output->acti_vals[i];
			} else {
				errors[v_index] = 0.0;
			}
		}

		network->backprop(errors);
	}

	
}
