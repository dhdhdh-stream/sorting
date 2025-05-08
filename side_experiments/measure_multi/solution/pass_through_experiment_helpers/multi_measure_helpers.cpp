#include "pass_through_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "abstract_node.h"
#include "constants.h"
#include "problem.h"
#include "scope.h"
#include "solution_helpers.h"

using namespace std;

void PassThroughExperiment::multi_measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		PassThroughExperimentHistory* history) {
	if (history->is_active) {
		for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
			if (this->step_types[s_index] == STEP_TYPE_ACTION) {
				problem->perform_action(this->actions[s_index]);
			} else {
				ScopeHistory* inner_scope_history = new ScopeHistory(this->scopes[s_index]);
				this->scopes[s_index]->activate(problem,
					run_helper,
					inner_scope_history);
				delete inner_scope_history;
			}

			run_helper.num_actions += 2;
		}

		curr_node = this->exit_next_node;
	}
}

void PassThroughExperiment::multi_measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories[this];

	vector<pair<AbstractExperiment*,bool>> curr_influence_indexes;
	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
			it != run_helper.experiment_histories.end(); it++) {
		if (it->first != this) {
			curr_influence_indexes.push_back({it->first, it->second->is_active});
		}
	}

	if (history->is_active) {
		this->new_target_vals.push_back(target_val);
		this->new_influence_indexes.push_back(curr_influence_indexes);
	} else {
		this->existing_target_vals.push_back(target_val);
		this->existing_influence_indexes.push_back(curr_influence_indexes);
	}
}

void PassThroughExperiment::multi_measure_calc() {
	double existing_sum_target_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		existing_sum_target_vals += this->existing_target_vals[h_index];
	}
	double existing_average_target_val = existing_sum_target_vals / (int)this->existing_target_vals.size();

	double existing_adjust;
	{
		map<AbstractExperiment*, pair<int,int>> sum_counts;
		for (int h_index = 0; h_index < (int)this->existing_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<AbstractExperiment*,bool> influence = this->existing_influence_indexes[h_index][i_index];
				map<AbstractExperiment*, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				if (influence.second) {
					it->second.second++;
				} else {
					it->second.first++;
				}
			}
		}

		map<AbstractExperiment*, int> influence_mapping;
		for (map<AbstractExperiment*, pair<int,int>>::iterator it = sum_counts.begin();
				it != sum_counts.end(); it++) {
			int sum_count = it->second.first + it->second.second;
			if (sum_count > INFLUENCE_MIN_NUM) {
				double curr_percentage = (double)it->second.second / (double)sum_count;
				double curr_standard_deviation = sqrt(curr_percentage * (1.0 - curr_percentage));
				if (curr_standard_deviation < MIN_STANDARD_DEVIATION) {
					curr_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				double t_score = ((1.0 / 3.0) - curr_percentage)
					/ curr_standard_deviation / sqrt(sum_count);
				if (abs(t_score) > 0.674) {
					this->improvement = -1.0;
					return;
				}

				influence_mapping[it->first] = 1 + (int)influence_mapping.size();
			}
		}

		Eigen::MatrixXd inputs((int)this->existing_target_vals.size(), 1 + influence_mapping.size());
		for (int i_index = 0; i_index < (int)this->existing_target_vals.size(); i_index++) {
			for (int m_index = 0; m_index < 1 + (int)influence_mapping.size(); m_index++) {
				inputs(i_index, m_index) = 0.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<AbstractExperiment*,bool> influence = this->existing_influence_indexes[h_index][i_index];
				if (influence.second) {
					inputs(h_index, influence_mapping[influence.first]) = 1.0;
				}
			}
		}

		Eigen::VectorXd outputs((int)this->existing_target_vals.size());
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			outputs(h_index) = this->existing_target_vals[h_index] - existing_average_target_val;
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			this->improvement = -1.0;
			return;
		}

		// for (int w_index = 0; w_index < (int)weights.size(); w_index++) {
		// 	cout << w_index << ": " << weights[w_index] << endl;
		// }

		if (abs(weights[0]) > 10000.0) {
			this->improvement = -1.0;
			return;
		}

		existing_adjust = weights(0);
	}

	double new_sum_target_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		new_sum_target_vals += this->new_target_vals[h_index];
	}
	double new_average_target_val = new_sum_target_vals / (int)this->new_target_vals.size();

	double new_adjust;
	{
		map<AbstractExperiment*, pair<int,int>> sum_counts;
		for (int h_index = 0; h_index < (int)this->new_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<AbstractExperiment*,bool> influence = this->new_influence_indexes[h_index][i_index];
				map<AbstractExperiment*, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				if (influence.second) {
					it->second.second++;
				} else {
					it->second.first++;
				}
			}
		}

		map<AbstractExperiment*, int> influence_mapping;
		for (map<AbstractExperiment*, pair<int,int>>::iterator it = sum_counts.begin();
				it != sum_counts.end(); it++) {
			int sum_count = it->second.first + it->second.second;
			if (sum_count > INFLUENCE_MIN_NUM) {
				double curr_percentage = (double)it->second.second / (double)sum_count;
				double curr_standard_deviation = sqrt(curr_percentage * (1.0 - curr_percentage));
				if (curr_standard_deviation < MIN_STANDARD_DEVIATION) {
					curr_standard_deviation = MIN_STANDARD_DEVIATION;
				}

				double t_score = ((1.0 / 3.0) - curr_percentage)
					/ curr_standard_deviation / sqrt(sum_count);
				if (abs(t_score) > 0.674) {
					this->improvement = -1.0;
					return;
				}

				influence_mapping[it->first] = 1 + (int)influence_mapping.size();
			}
		}

		Eigen::MatrixXd inputs((int)this->new_target_vals.size(), 1 + influence_mapping.size());
		for (int i_index = 0; i_index < (int)this->new_target_vals.size(); i_index++) {
			for (int m_index = 0; m_index < 1 + (int)influence_mapping.size(); m_index++) {
				inputs(i_index, m_index) = 0.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<AbstractExperiment*,bool> influence = this->new_influence_indexes[h_index][i_index];
				if (influence.second) {
					inputs(h_index, influence_mapping[influence.first]) = 1.0;
				}
			}
		}

		Eigen::VectorXd outputs((int)this->new_target_vals.size());
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			outputs(h_index) = this->new_target_vals[h_index] - new_average_target_val;
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			this->improvement = -1.0;
			return;
		}

		// for (int w_index = 0; w_index < (int)weights.size(); w_index++) {
		// 	cout << w_index << ": " << weights[w_index] << endl;
		// }

		if (abs(weights[0]) > 10000.0) {
			this->improvement = -1.0;
			return;
		}

		new_adjust = weights(0);
	}

	double existing_score = existing_average_target_val + existing_adjust;
	double new_score = new_average_target_val + new_adjust;

	this->improvement = new_score - existing_score;

	cout << "existing_average_target_val: " << existing_average_target_val << endl;
	cout << "new_average_target_val: " << new_average_target_val << endl;
}
