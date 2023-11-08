#include "pass_through_experiment.h"

using namespace std;

void PassThroughExperiment::measure_existing_score_activate(
		vector<ContextLayer>& context) {
	context[context.size() - this->scope_context.size()]
		.scope_history->inner_pass_through_experiment = this;

	for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
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

	this->i_input_state_vals_histories.push_back(vector<map<int, StateStatus>>{context.back().input_state_vals});
	this->i_local_state_vals_histories.push_back(vector<map<int, StateStatus>>{context.back().local_state_vals});
	this->i_temp_state_vals_histories.push_back(vector<map<State*, StateStatus>>{context.back().temp_state_vals});

	PassThroughExperimentOverallHistory* history = (PassThroughExperimentOverallHistory*)run_helper.experiment_history;
	history->instance_count++;
}

void possible_exits_helper(set<pair<int, AbstractNode*>>& s_possible_exits,
						   int curr_exit_depth,
						   ScopeHistory* scope_history) {
	for (int i_index = scope_history->experiment_iter_index + 1; i_index < (int)scope_history->node_histories.size(); i_index++) {
		for (int h_index = scope_history->experiment_node_index + 1; h_index < (int)scope_history->node_histories[i_index].size(); h_index++) {
			s_possible_exits.insert({curr_exit_depth, scope_history->node_histories[i_index][h_index]->node});
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
		RunHelper& run_helper,
		PassThroughExperimentOverallHistory* history) {
	this->o_target_val_histories.push_back(target_val);

	for (int i_index = 0; i_index < (int)history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val);
	}

	if (!run_helper.exceeded_depth) {
		if (run_helper.max_depth > solution->max_depth) {
			solution->max_depth = run_helper.max_depth;

			if (solution->max_depth < 50) {
				solution->depth_limit = solution->max_depth + 10;
			} else {
				solution->depth_limit = (int)(1.2*(double)solution->max_depth);
			}
		}
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
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (map<int, StateStatus>::iterator m_it = this->i_input_state_vals_histories[i_index][0].begin();
					m_it != this->i_input_state_vals_histories[i_index][0].end(); m_it++) {
				map<int, vector<double>>::iterator p_it = p_input_state_vals.find(m_it->first);
				if (p_it == p_input_state_vals.end()) {
					p_it = p_input_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
				}

				StateNetwork* last_network = m_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (m_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					p_it->second[i_index] = normalized;
				} else {
					p_it->second[i_index] = m_it->second.val;
				}
			}
		}

		map<int, vector<double>> p_local_state_vals;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (map<int, StateStatus>::iterator m_it = this->i_local_state_vals_histories[i_index][0].begin();
					m_it != this->i_local_state_vals_histories[i_index][0].end(); m_it++) {
				map<int, vector<double>>::iterator p_it = p_local_state_vals.find(m_it->first);
				if (p_it == p_local_state_vals.end()) {
					p_it = p_local_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
				}

				StateNetwork* last_network = m_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (m_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					p_it->second[i_index] = normalized;
				} else {
					p_it->second[i_index] = m_it->second.val;
				}
			}
		}

		map<State*, vector<double>> p_temp_state_vals;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (map<State*, StateStatus>::iterator m_it = this->i_temp_state_vals_histories[i_index][0].begin();
					m_it != this->i_temp_state_vals_histories[i_index][0].end(); m_it++) {
				map<State*, vector<double>>::iterator p_it = p_temp_state_vals.find(m_it->first);
				if (p_it == p_temp_state_vals.end()) {
					p_it = p_temp_state_vals.insert({m_it->first, vector<double>(num_instances, 0.0)}).first;
				}

				StateNetwork* last_network = m_it->second.last_network;
				if (last_network != NULL) {
					double normalized = (m_it->second.val - last_network->ending_mean)
						/ last_network->ending_standard_deviation;
					p_it->second[i_index] = normalized;
				} else {
					p_it->second[i_index] = m_it->second.val;
				}
			}
		}

		Eigen::MatrixXd inputs(num_instances, (int)p_input_state_vals.size() + (int)p_local_state_vals.size() + (int)p_temp_state_vals.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			int s_index = 0;

			for (map<int, vector<double>>::iterator it = p_input_state_vals.begin();
					it != p_input_state_vals.end(); it++) {
				inputs(i_index, s_index) = it->second[i_index];
				s_index++;
			}

			for (map<int, vector<double>>::iterator it = p_local_state_vals.begin();
					it != p_local_state_vals.end(); it++) {
				inputs(i_index, s_index) = it->second[i_index];
				s_index++;
			}

			for (map<State*, vector<double>>::iterator it = p_temp_state_vals.begin();
					it != p_temp_state_vals.end(); it++) {
				inputs(i_index, s_index) = it->second[i_index];
				s_index++;
			}
		}

		Eigen::VectorXd outputs(num_instances);
		for (int i_index = 0; i_index < num_instances; i_index++) {
			outputs(i_index) = this->i_target_val_histories[i_index] - this->existing_average_score;
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
		for (int i_index = 0; i_index < num_instances; i_index++) {
			obs_experiment_target_vals[i_index] = diffs(i_index);
		}

		existing_obs_experiment(this,
								solution->scopes[this->scope_context[0]],
								i_scope_histories,
								obs_experiment_target_vals);

		set<pair<int, AbstractNode*>> s_possible_exits;
		for (int i_index = 0; i_index < num_instances; i_index++) {
			possible_exits_helper(s_possible_exits,
								  this->scope_context.size()-1,
								  this->i_scope_histories[i_index]);
		}
		this->possible_exits.reserve(s_possible_exits.size());
		for (set<pair<int, AbstractNode*>>::iterator it = s_possible_exits.begin();
				it != s_possible_exits.end(); it++) {
			this->possible_exits.push_back(*it);
		}
		for (int c_index = 0; c_index < (int)this->scope_context.size(); c_index++) {
			this->possible_exits.push_back({c_index, NULL});
		}

		this->o_target_val_histories.clear();
		for (int i_index = 0; i_index < (int)this->i_scope_histories.size(); i_index++) {
			delete this->i_scope_histories[i_index];
		}
		this->i_scope_histories.clear();
		this->i_input_state_vals_histories.clear();
		this->i_local_state_vals_histories.clear();
		this->i_temp_state_vals_histories.clear();
		this->i_target_val_histories.clear();

		this->state = PASS_THROUGH_EXPERIMENT_STATE_EXPLORE;
		this->state_iter = 0;
	}
}
