#include "experiment.h"

#include <iostream>
#include <Eigen/Dense>

#include "action.h"
#include "constants.h"
#include "globals.h"
#include "transform.h"
#include "world_model.h"
#include "world_state.h"

using namespace std;

const int TRAIN_NUM_SAMPLES = 2000;

void Experiment::train_activate(WorldState*& curr_state) {
	curr_state = this->experiment_states[0];
}

void Experiment::train_backprop(double target_val,
								WorldState* ending_state,
								vector<double>& ending_state_vals) {
	map<WorldState*, int>::iterator it = this->experiment_state_reverse_mapping.find(ending_state);
	if (it != this->experiment_state_reverse_mapping.end()) {
		this->ending_vals[it->second].push_back(target_val);
		this->ending_state_vals[it->second].push_back(ending_state_vals);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_NUM_SAMPLES) {
		this->ending_val_averages = vector<double>(this->experiment_states.size());
		for (int a_index = 0; a_index < (int)this->experiment_states.size(); a_index++) {
			if (this->ending_vals[a_index].size() > 0) {
				double sum_vals = 0.0;
				for (int d_index = 0; d_index < (int)this->ending_vals[a_index].size(); d_index++) {
					sum_vals += this->ending_vals[a_index][d_index];
				}
				this->ending_val_averages[a_index] = sum_vals / this->ending_vals[a_index].size();
			}
		}

		this->ending_state_val_impacts = vector<vector<double>>(this->experiment_states.size(),
			vector<double>(world_model->num_states, 0.0));
		for (int a_index = 0; a_index < (int)this->experiment_states.size(); a_index++) {
			if (this->ending_vals[a_index].size() > 0 && world_model->num_states > 0) {
				Eigen::MatrixXd inputs(this->ending_vals[a_index].size(), world_model->num_states);
				for (int d_index = 0; d_index < (int)this->ending_vals[a_index].size(); d_index++) {
					for (int s_index = 0; s_index < world_model->num_states; s_index++) {
						inputs(d_index, s_index) = this->ending_state_vals[a_index][d_index][s_index];
					}
				}

				Eigen::VectorXd outputs(this->ending_vals[a_index].size());
				for (int d_index = 0; d_index < (int)this->ending_vals[a_index].size(); d_index++) {
					outputs(d_index) = this->ending_vals[a_index][d_index] - this->ending_val_averages[a_index];
				}

				Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				for (int s_index = 0; s_index < world_model->num_states; s_index++) {
					this->ending_state_val_impacts[a_index][s_index] = weights(s_index);
				}
			}
		}

		for (int a_index = 0; a_index < (int)this->experiment_states.size(); a_index++) {
			int num_instances = (int)this->experiment_states[a_index]->hook_obs.size();

			double sum_obs = 0.0;
			for (int d_index = 0; d_index < num_instances; d_index++) {
				sum_obs += this->experiment_states[a_index]->hook_obs[d_index];
			}
			this->experiment_states[a_index]->obs_average = sum_obs / num_instances;

			this->experiment_states[a_index]->state_obs_impacts = vector<double>(world_model->num_states, 0.0);
			if (num_instances > 0 && world_model->num_states > 0) {
				Eigen::MatrixXd inputs(num_instances, world_model->num_states);
				for (int d_index = 0; d_index < num_instances; d_index++) {
					for (int s_index = 0; s_index < world_model->num_states; s_index++) {
						inputs(d_index, s_index) = this->experiment_states[a_index]->hook_state_vals[d_index][s_index];
					}
				}

				Eigen::VectorXd outputs(num_instances);
				for (int d_index = 0; d_index < num_instances; d_index++) {
					outputs(d_index) = this->experiment_states[a_index]->hook_obs[d_index]
						- this->experiment_states[a_index]->obs_average;
				}

				Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
				for (int s_index = 0; s_index < world_model->num_states; s_index++) {
					this->experiment_states[a_index]->state_obs_impacts[s_index] = weights(s_index);
				}
			}

			this->experiment_states[a_index]->state_averages = vector<double>(world_model->num_states);
			for (int s_index = 0; s_index < world_model->num_states; s_index++) {
				double sum_vals = 0.0;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					sum_vals += this->experiment_states[a_index]->hook_state_vals[d_index][s_index];
				}
				this->experiment_states[a_index]->state_averages[s_index] = sum_vals / num_instances;
			}

			for (map<Action*, vector<vector<pair<double, double>>>>::iterator it = this->experiment_states[a_index]->hook_impact_vals.begin();
					it != this->experiment_states[a_index]->hook_impact_vals.end(); it++) {
				this->experiment_states[a_index]->action_impacts[it->first] = vector<Transform*>(it->first->num_inputs, NULL);
				for (int i_index = 0; i_index < it->first->num_inputs; i_index++) {
					int num_impact_instances = it->second[i_index].size();
					if (num_impact_instances > 0) {
						Eigen::MatrixXd inputs(num_impact_instances, 2);
						for (int d_index = 0; d_index < num_impact_instances; d_index++) {
							inputs(d_index, 0) = it->second[i_index][d_index].first;
							inputs(d_index, 1) = 1.0;
						}

						Eigen::VectorXd outputs(num_impact_instances);
						for (int d_index = 0; d_index < num_impact_instances; d_index++) {
							outputs(d_index) = it->second[i_index][d_index].second;
						}

						Eigen::VectorXd weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
						Transform* transform = new Transform(weights(0), weights(1));
						this->experiment_states[a_index]->action_impacts[it->first][i_index] = transform;
					}
				}
			}
		}

		for (int a_index = 0; a_index < (int)this->experiment_states.size(); a_index++) {
			this->experiment_states[a_index]->experiment_hook = NULL;
			this->experiment_states[a_index]->hook_obs.clear();
			this->experiment_states[a_index]->hook_state_vals.clear();
			this->experiment_states[a_index]->hook_impact_vals.clear();
		}

		this->state = EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
