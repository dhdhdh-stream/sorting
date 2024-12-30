#include "branch_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

using namespace std;

const int TRAIN_NEW_NUM_DATAPOINTS = 2000;

bool BranchExperiment::train_new_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	run_helper.num_actions++;

	this->num_instances_until_target--;

	if (this->num_instances_until_target <= 0) {
		run_helper.has_explore = true;

		history->instance_count++;

		map<pair<int,int>, double> factors;
		gather_factors(run_helper,
					   scope_history,
					   factors);
		double sum_vals = this->existing_average_score;
		for (int f_index = 0; f_index < (int)this->existing_factor_ids.size(); f_index++) {
			map<pair<int,int>, double>::iterator it = factors.find(this->existing_factor_ids[f_index]);
			if (it != factors.end()) {
				sum_vals += this->existing_factor_weights[f_index] * it->second;
			}
		}
		history->existing_predicted_scores.push_back(sum_vals);

		vector<double> input_vals(this->new_inputs.size());
		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			fetch_input_helper(run_helper,
							   scope_history,
							   this->new_inputs[i_index],
							   0,
							   input_vals[i_index]);
		}
		this->input_histories.push_back(input_vals);

		this->factor_histories.push_back(factors);

		for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
			if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->best_actions[s_index]);
			} else {
				context.back().node_id = -1;

				this->best_scopes[s_index]->activate(problem,
					context,
					run_helper);
			}

			run_helper.num_actions += 2;
		}

		uniform_int_distribution<int> until_distribution(0, 2*((int)this->node_context->average_instances_per_run-1));
		this->num_instances_until_target = 1 + until_distribution(generator);

		return true;
	}

	return false;
}

void BranchExperiment::train_new_backprop(
		double target_val,
		RunHelper& run_helper) {
	BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

	for (int i_index = 0; i_index < history->instance_count; i_index++) {
		this->i_target_val_histories.push_back(target_val - history->existing_predicted_scores[i_index]);
	}

	this->state_iter++;
	if (this->state_iter >= TRAIN_NEW_NUM_DATAPOINTS) {
		{
			default_random_engine generator_copy = generator;
			shuffle(this->input_histories.begin(), this->input_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->factor_histories.begin(), this->factor_histories.end(), generator_copy);
		}
		{
			default_random_engine generator_copy = generator;
			shuffle(this->i_target_val_histories.begin(), this->i_target_val_histories.end(), generator_copy);
		}

		int num_instances = (int)this->i_target_val_histories.size();
		int num_train_instances = (double)num_instances * (1.0 - TEST_SAMPLES_PERCENTAGE);
		int num_test_instances = num_instances - num_train_instances;

		map<pair<int,int>, double> sum_factor_impacts;
		for (int i_index = 0; i_index < num_train_instances; i_index++) {
			for (map<pair<int,int>, double>::iterator it = this->factor_histories[i_index].begin();
					it != this->factor_histories[i_index].end(); it++) {
				map<pair<int,int>, double>::iterator s_it = sum_factor_impacts.find(it->first);
				if (s_it == sum_factor_impacts.end()) {
					sum_factor_impacts[it->first] = abs(it->second);
				} else {
					s_it->second += abs(it->second);
				}
			}
		}

		vector<double> remaining_scores(num_instances);

		int num_factors = (int)sum_factor_impacts.size();
		if (num_factors > 0) {
			double sum_offset = 0.0;
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				sum_offset += abs(this->i_target_val_histories[i_index]);
			}
			double average_offset = sum_offset / num_train_instances;

			map<pair<int,int>, int> factor_mapping;
			for (map<pair<int,int>, double>::iterator it = sum_factor_impacts.begin();
					it != sum_factor_impacts.end(); it++) {
				int index = (int)factor_mapping.size();
				factor_mapping[it->first] = index;
			}

			Eigen::MatrixXd inputs(num_train_instances, num_factors);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				for (int f_index = 0; f_index < num_factors; f_index++) {
					inputs(i_index, f_index) = 0.0;
				}

				for (map<pair<int,int>, double>::iterator it = this->factor_histories[i_index].begin();
						it != this->factor_histories[i_index].end(); it++) {
					inputs(i_index, factor_mapping[it->first]) = it->second;
				}
			}

			Eigen::VectorXd outputs(num_train_instances);
			for (int i_index = 0; i_index < num_train_instances; i_index++) {
				outputs(d_index) = this->i_target_val_histories[i_index];
			}

			Eigen::VectorXd weights;
			try {
				weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
			} catch (std::invalid_argument &e) {
				cout << "Eigen error" << endl;
				weights = Eigen::VectorXd(num_factors);
				for (int f_index = 0; f_index < num_factors; f_index++) {
					weights(f_index) = 0.0;
				}
			}

			double impact_threshold = average_offset / num_factors * FACTOR_IMPACT_THRESHOLD;

			for (map<pair<int,int>, double>::iterator it = sum_factor_impacts.begin();
					it != sum_factor_impacts.end(); it++) {
				if (abs(weights(factor_mapping[it->first])) * sum_factor_impacts[it->first] > impact_threshold) {
					this->new_factor_ids.push_back(it->first);
					this->new_factor_weights.push_back(weights(factor_mapping[it->first]));
				}
			}

			for (int i_index = 0; i_index < num_instances; i_index++) {
				double sum_score = 0.0;
				for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
					map<pair<int,int>, double>::iterator it = this->factor_histories[i_index]
						.find(this->new_factor_ids[f_index]);
					if (it != this->factor_histories[i_index].end()) {
						sum_score += this->new_factor_weights[f_index] * it->double;
					}
				}

				remaining_scores[i_index] = this->i_target_val_histories[i_index] - sum_score;
			}
		} else {
			for (int i_index = 0; i_index < num_instances; i_index++) {
				remaining_scores[i_index] = this->i_target_val_histories[i_index];
			}
		}



	}
}
