#include "new_scope_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int NEW_SCOPE_NUM_DATAPOINTS = 2;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 5;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 10;
#else
const int NEW_SCOPE_NUM_DATAPOINTS = 40;
const int NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS = 400;
const int NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void NewScopeExperiment::calc_improve_helper(bool& is_success,
											 double& curr_improvement) {
	map<int, pair<int,int>> sum_counts;
	for (int h_index = 0; h_index < (int)this->existing_influence_indexes.size(); h_index++) {
		for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
			pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
			if (influence.second) {
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				it->second.first++;
			}
		}
	}
	for (int h_index = 0; h_index < (int)this->new_influence_indexes.size(); h_index++) {
		for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
			pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
			if (influence.second) {
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				it->second.second++;
			}
		}
	}

	map<int, int> influence_mapping;
	for (map<int, pair<int,int>>::iterator it = sum_counts.begin();
			it != sum_counts.end(); it++) {
		if (it->second.first > INFLUENCE_MIN_NUM
				&& it->second.second > INFLUENCE_MIN_NUM) {
			influence_mapping[it->first] = (int)influence_mapping.size();
		}
	}

	double existing_sum_target_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		existing_sum_target_vals += this->existing_target_vals[h_index];
	}
	double existing_average_target_val = existing_sum_target_vals / (int)this->existing_target_vals.size();

	double new_sum_target_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		new_sum_target_vals += this->new_target_vals[h_index];
	}
	double new_average_target_val = new_sum_target_vals / (int)this->new_target_vals.size();

	if (influence_mapping.size() > 0) {
		Eigen::MatrixXd existing_inputs((int)this->existing_target_vals.size(), influence_mapping.size());
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			for (int m_index = 0; m_index < (int)influence_mapping.size(); m_index++) {
				existing_inputs(h_index, m_index) = 0.0;
			}
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
				if (influence.second) {
					map<int, int>::iterator it = influence_mapping.find(influence.first);
					if (it != influence_mapping.end()) {
						existing_inputs(h_index, it->second) = 1.0;
					}
				}
			}
		}

		Eigen::VectorXd existing_outputs((int)this->existing_target_vals.size());
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			existing_outputs(h_index) = this->existing_target_vals[h_index] - existing_average_target_val;
		}

		Eigen::VectorXd existing_weights;
		try {
			existing_weights = existing_inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(existing_outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			is_success = false;
			return;
		}

		Eigen::MatrixXd new_inputs((int)this->new_target_vals.size(), influence_mapping.size());
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			for (int m_index = 0; m_index < (int)influence_mapping.size(); m_index++) {
				new_inputs(h_index, m_index) = 0.0;
			}
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
				if (influence.second) {
					map<int, int>::iterator it = influence_mapping.find(influence.first);
					if (it != influence_mapping.end()) {
						new_inputs(h_index, it->second) = 1.0;
					}
				}
			}
		}

		Eigen::VectorXd new_outputs((int)this->new_target_vals.size());
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			new_outputs(h_index) = this->new_target_vals[h_index] - new_average_target_val;
		}

		Eigen::VectorXd new_weights;
		try {
			new_weights = new_inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(new_outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			is_success = false;
			return;
		}

		double sum_variance = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			sum_variance += (this->existing_target_vals[h_index] - existing_average_target_val)
				* (this->existing_target_vals[h_index] - existing_average_target_val);
		}
		double standard_deviation = sqrt(sum_variance / (int)this->existing_target_vals.size());

		double max_impact = INFLUENCE_MAX_PERCENTAGE * standard_deviation;

		// temp
		cout << "standard_deviation: " << standard_deviation << endl;

		vector<bool> is_correlated(influence_mapping.size());
		for (int w_index = 0; w_index < (int)influence_mapping.size(); w_index++) {
			// temp
			cout << w_index << endl;
			cout << "existing_weights(w_index): " << existing_weights(w_index) << endl;
			cout << "new_weights(w_index): " << new_weights(w_index) << endl;

			if (abs(existing_weights(w_index) - new_weights(w_index)) > max_impact) {
				is_correlated[w_index] = true;
			} else {
				is_correlated[w_index] = false;
			}
		}

		double sum_existing_score = 0.0;
		int existing_count = 0;
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			bool is_valid = true;
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
				if (influence.second) {
					map<int, int>::iterator it = influence_mapping.find(influence.first);
					if (it != influence_mapping.end()) {
						if (is_correlated[it->second]) {
							is_valid = false;
							break;
						}
					}
				}
			}

			if (is_valid) {
				sum_existing_score += this->existing_target_vals[h_index];
				existing_count++;
			}
		}

		double sum_new_score = 0.0;
		int new_count = 0;
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			bool is_valid = true;
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
				if (influence.second) {
					map<int, int>::iterator it = influence_mapping.find(influence.first);
					if (it != influence_mapping.end()) {
						if (is_correlated[it->second]) {
							is_valid = false;
							break;
						}
					}
				}
			}

			if (is_valid) {
				sum_new_score += this->new_target_vals[h_index];
				new_count++;
			}
		}

		int min_samples_needed = INFLUENCE_VALID_MIN_PERCENTAGE * (int)this->new_target_vals.size();
		if (existing_count > min_samples_needed
				&& new_count > min_samples_needed) {
			double existing_score = sum_existing_score / existing_count;
			double new_score = sum_new_score / new_count;

			is_success = true;
			curr_improvement = new_score - existing_score;
		} else {
			is_success = false;
		}
	} else {
		is_success = true;
		curr_improvement = new_average_target_val - existing_average_target_val;
	}
}

void NewScopeExperiment::test_backprop(
		double target_val,
		bool is_return,
		RunHelper& run_helper,
		NewScopeExperimentHistory* history) {
	if (this->test_location_state == LOCATION_STATE_CHECK_LOCATION) {
		if (history->is_active) {
			if (is_return) {
				this->test_location_state = LOCATION_STATE_MEASURE;
			} else {
				this->test_location_start->experiment = NULL;
				this->test_location_start = NULL;

				if (this->generalize_iter == -1
						&& this->successful_location_starts.size() == 0) {
					this->result = EXPERIMENT_RESULT_FAIL;
					/**
					 * - only continue if first succeeds
					 */
				} else {
					this->generalize_iter++;
				}
			}
		}
	} else {
		if (is_return) {
			vector<pair<int,bool>> curr_influence_indexes;
			for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
					it != run_helper.experiment_histories.end(); it++) {
				if (it->first != this) {
					curr_influence_indexes.push_back({it->first->multi_index, it->second->is_active});
				}
			}

			if (history->is_active) {
				this->new_target_vals.push_back(target_val);
				this->new_influence_indexes.push_back(curr_influence_indexes);

				bool is_fail = false;
				switch (this->test_location_state) {
				case LOCATION_STATE_MEASURE:
					if ((int)this->new_target_vals.size() == NEW_SCOPE_NUM_DATAPOINTS) {
						bool is_success;
						double curr_improvement;
						calc_improve_helper(is_success,
											curr_improvement);

						this->existing_target_vals.clear();
						this->existing_influence_indexes.clear();
						this->new_target_vals.clear();
						this->new_influence_indexes.clear();

						#if defined(MDEBUG) && MDEBUG
						if (rand()%2 == 0) {
						#else
						if (!is_success || curr_improvement <= 0.0) {
						#endif /* MDEBUG */
							is_fail = true;
						} else {
							this->test_location_state = LOCATION_STATE_VERIFY_1ST;
						}
					}
					break;
				case LOCATION_STATE_VERIFY_1ST:
					if ((int)this->new_target_vals.size() == NEW_SCOPE_VERIFY_1ST_NUM_DATAPOINTS) {
						bool is_success;
						double curr_improvement;
						calc_improve_helper(is_success,
											curr_improvement);

						this->existing_target_vals.clear();
						this->existing_influence_indexes.clear();
						this->new_target_vals.clear();
						this->new_influence_indexes.clear();

						#if defined(MDEBUG) && MDEBUG
						if (rand()%2 == 0) {
						#else
						if (!is_success || curr_improvement <= 0.0) {
						#endif /* MDEBUG */
							is_fail = true;
						} else {
							this->test_location_state = LOCATION_STATE_VERIFY_2ND;
						}
					}
					break;
				case LOCATION_STATE_VERIFY_2ND:
					if ((int)this->new_target_vals.size() == NEW_SCOPE_VERIFY_2ND_NUM_DATAPOINTS) {
						bool is_success;
						double curr_improvement;
						calc_improve_helper(is_success,
											curr_improvement);

						this->existing_target_vals.clear();
						this->existing_influence_indexes.clear();
						this->new_target_vals.clear();
						this->new_influence_indexes.clear();

						#if defined(MDEBUG) && MDEBUG
						if (rand()%2 == 0) {
						#else
						if (!is_success || curr_improvement <= 0.0) {
						#endif /* MDEBUG */
							is_fail = true;
						} else {
							this->improvement = curr_improvement;

							this->successful_location_starts.push_back(this->test_location_start);
							this->successful_location_is_branch.push_back(this->test_location_is_branch);
							this->successful_location_exits.push_back(this->test_location_exit);

							this->test_location_start = NULL;

							this->generalize_iter++;
						}
					}
					break;
				}

				if (is_fail) {
					this->test_location_start->experiment = NULL;
					this->test_location_start = NULL;

					if (this->generalize_iter == -1
							&& this->successful_location_starts.size() == 0) {
						this->result = EXPERIMENT_RESULT_FAIL;
						/**
						 * - only continue if first succeeds
						 */
					} else {
						this->generalize_iter++;
					}
				}

				if (this->successful_location_starts.size() >= NEW_SCOPE_NUM_LOCATIONS) {
					cout << "NewScopeExperiment success" << endl;
					cout << "this->improvement: " << this->improvement << endl;

					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else if (this->generalize_iter >= NEW_SCOPE_NUM_GENERALIZE_TRIES) {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			} else {
				this->existing_target_vals.push_back(target_val);
				this->existing_influence_indexes.push_back(curr_influence_indexes);
			}
		}
	}
}
