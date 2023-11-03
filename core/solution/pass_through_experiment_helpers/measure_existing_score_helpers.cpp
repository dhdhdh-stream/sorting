#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::measure_existing_score_activate(
		vector<ContextLayer>& context) {
	context[context.size() - this->scope_context.size()]
		.scope_history->inner_pass_through_experiment = this;

	for (int c_index = 0; c_index < this->scope_context.size(); c_index++) {
		ScopeHistory* scope_history = context[context.size()-1 - c_index].scope_history;
		scope_history->experiment_iter_index = (int)scope_history.size()-1;
		scope_history->experiment_node_index = (int)scope_history.back().size()-1;
	}
}

void PassThroughExperiment::measure_existing_score_parent_scope_end_activate(
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* parent_scope_history) {
	this->i_scope_histories.push_back(new ScopeHistory(parent_scope_history));

	this->i_input_state_vals_histories.push_back(context.back().input_state_vals);
	this->i_local_state_vals_histories.push_back(context.back().local_state_vals);
	this->i_temp_state_vals_histories.push_back(context.back().temp_state_vals);

	PassThroughExperimentOverallHistory* history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	history->instance_count++;
}

void PassThroughExperiment::possible_exits_helper(
		set<pair<int, int>>& s_possible_exits,
		int curr_exit_depth,
		ScopeHistory* scope_history) {
	for (int i_index = scope_history->experiment_iter_index + 1; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = scope_history->experiment_node_index + 1; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			int node_id = scope_history->node_histories[i_index][h_index]->node->id;
			s_possible_exits.insert({curr_exit_depth, node_id});
		}
	}

	if (curr_exit_depth > 0) {
		ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)scope_history
			->node_histories[scope_history->experiment_iter_index][scope_history->experiment_node_index];
		possible_exits_helper(curr_exit_depth-1,
							  scope_node_history->inner_scope_history);
	}
}

void PassThroughExperiment::measure_existing_score_backprop(
		double target_val,
		PassThroughExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	if (this->o_target_val_histories.size() >= solution->curr_num_datapoints) {
		double sum_scores = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_scores += this->o_target_val_histories[d_index];
		}
		this->existing_average_score = sum_scores / solution->curr_num_datapoints;

		double sum_score_variance = 0.0;
		for (int d_index = 0; d_index < solution->curr_num_datapoints; d_index++) {
			sum_score_variance += (this->o_target_val_histories[d_index] - this->existing_average_score) * (this->o_target_val_histories[d_index] - this->existing_average_score);
		}
		this->existing_score_variance = sum_score_variance / solution->curr_num_datapoints;

		int num_instances = (int)this->i_target_val_histories.size();

		map<int, vector<double>> p_input_state_vals;
		{
			int d_index = 0;
			for (list<map<int, StateStatus>>::iterator l_it = this->i_input_state_vals_histories.begin();
					l_it != this->i_input_state_vals_histories.end(); l_it++) {
				for (map<int, StateStatus>::iterator m_it = (*l_it).begin(); m_it != (*l_it).end(); m_it++) {
					map<int, vector<double>>::iterator p_it = p_input_state_vals.find(m_it->first);
					if (p_it == p_input_state_vals.end()) {
						p_it = p_input_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[d_index] = normalized;
					} else {
						p_it->second[d_index] = m_it->second.val;
					}
				}

				d_index++;
			}
		}

		map<int, vector<double>> p_local_state_vals;
		{
			int d_index = 0;
			for (list<map<int, StateStatus>>::iterator l_it = this->i_local_state_vals_histories.begin();
					l_it != this->i_local_state_vals_histories.end(); l_it++) {
				for (map<int, StateStatus>::iterator m_it = (*l_it).begin(); m_it != (*l_it).end(); m_it++) {
					map<int, vector<double>>::iterator p_it = p_local_state_vals.find(m_it->first);
					if (p_it == p_local_state_vals.end()) {
						p_it = p_local_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[d_index] = normalized;
					} else {
						p_it->second[d_index] = m_it->second.val;
					}
				}

				d_index++;
			}
		}

		map<State*, vector<double>> p_temp_state_vals;
		{
			int d_index = 0;
			for (list<map<State*, StateStatus>>::iterator l_it = this->i_temp_state_vals_histories.begin();
					l_it != this->i_temp_state_vals_histories.end(); l_it++) {
				for (map<State*, StateStatus>::iterator m_it = (*l_it).begin(); m_it != (*l_it).end(); m_it++) {
					map<State*, vector<double>>::iterator p_it = p_temp_state_vals.find(m_it->first);
					if (p_it == p_temp_state_vals.end()) {
						p_it = p_temp_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
					}

					StateNetwork* last_network = m_it->second.last_network;
					if (last_network != NULL) {
						double normalized = (m_it->second.val - last_network->ending_mean)
							/ last_network->ending_standard_deviation;
						p_it->second[d_index] = normalized;
					} else {
						p_it->second[d_index] = m_it->second.val;
					}
				}

				d_index++;
			}
		}

		Eigen::MatrixXd inputs(num_instances, (int)p_input_state_vals.size() + (int)p_local_state_vals.size() + (int)p_temp_state_vals.size());
		for (int d_index = 0; d_index < num_instances; d_index++) {
			int s_index = 0;

			for (map<int, vector<double>>::iterator it = p_input_state_vals.begin();
					it != p_input_state_vals.end(); it++) {
				inputs(d_index, s_index) = it->second[d_index];
				s_index++;
			}

			for (map<int, vector<double>>::iterator it = p_local_state_vals.begin();
					it != p_local_state_vals.end(); it++) {
				inputs(d_index, s_index) = it->second[d_index];
				s_index++;
			}

			for (map<State*, vector<double>>::iterator it = p_temp_state_vals.begin();
					it != p_temp_state_vals.end(); it++) {
				inputs(d_index, s_index) = it->second[d_index];
				s_index++;
			}
		}

		Eigen::VectorXd outputs(num_instances);
		{
			int d_index = 0;
			for (list<double>::iterator l_it = this->i_target_val_histories.begin();
					l_it != this->i_target_val_histories.end(); l_it++) {
				outputs(d_index) = (*l_it) - this->existing_average_score;

				d_index++;
			}
		}

		Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		{
			int s_index = 0;

			this->existing_input_state_weights.clear();
			for (map<int, vector<double>>::iterator it = p_input_state_vals.begin();
					it != p_input_state_vals.end(); it++) {
				this->existing_input_state_weights[it->first] = weights(s_index);
				s_index++;
			}

			this->existing_local_state_weights.clear();
			for (map<int, vector<double>>::iterator it = p_local_state_vals.begin();
					it != p_local_state_vals.end(); it++) {
				this->existing_local_state_weights[it->first] = weights(s_index);
				s_index++;
			}

			this->existing_temp_state_weights.clear();
			for (map<State*, vector<double>>::iterator it = p_temp_state_vals.begin();
					it != p_temp_state_vals.end(); it++) {
				this->existing_temp_state_weights[it->first] = weights(s_index);
				s_index++;
			}
		}

		Eigen::VectorXd predicted_scores = inputs * weights;
		Eigen::VectorXd diffs = outputs - predicted_scores;
		vector<double> obs_experiment_target_vals(num_instances);
		for (int d_index = 0; d_index < num_instances; d_index++) {
			obs_experiment_target_vals[d_index] = diffs(d_index);
		}

		existing_obs_experiment(this,
								solution->scopes[this->scope_context[0]],
								i_scope_histories,
								obs_experiment_target_vals);

		set<pair<int, int>> s_possible_exits;
		for (list<ScopeHistory*>::iterator it = this->i_scope_histories.begin();
				it != this->i_scope_histories.end(); it++) {
			possible_exits_helper(s_possible_exits,
								  this->scope_context.size()-1,
								  *it);
		}
		this->possible_exits.reserve(s_possible_exits.size());
		for (set<pair<int, int>>::iterator it = s_possible_exits.begin();
				it != s_possible_exits.end(); it++) {
			this->possible_exits.push_back(*it);
		}

		this->o_target_val_histories.clear();
		while (this->i_scope_histories.size() > 0) {
			delete this->i_scope_histories.front();
			this->i_scope_histories.pop_front();
		}
		this->i_input_state_vals_histories.clear();
		this->i_local_state_vals_histories.clear();
		this->i_temp_state_vals_histories.clear();
		this->i_target_val_histories.clear();

		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
