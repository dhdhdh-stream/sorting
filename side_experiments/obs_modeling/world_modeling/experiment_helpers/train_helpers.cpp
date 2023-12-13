#include "experiment.h"

#include <iostream>
#include <Eigen/Dense>

#include "constants.h"
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
			if (this->ending_vals[a_index].size() > 0) {
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
			this->experiment_states[a_index]->state_obs_impacts = vector<double>(world_model->num_states);
			for (int s_index = 0; s_index < world_model->num_states; s_index++) {
				this->experiment_states[a_index]->state_obs_impacts[s_index] = weights(s_index);
			}

			this->experiment_states[a_index]->state_averages = vector<double>(world_model->num_states);
			for (int s_index = 0; s_index < world_model->num_states; s_index++) {
				double sum_vals = 0.0;
				for (int d_index = 0; d_index < num_instances; d_index++) {
					sum_vals += this->experiment_states[a_index]->hook_state_vals[d_index][s_index];
				}
				this->experiment_states[a_index]->state_averages[s_index] = sum_vals / num_instances;
			}
		}

		this->state = EXPERIMENT_STATE_MEASURE;
		this->state_iter = 0;
	}
}
