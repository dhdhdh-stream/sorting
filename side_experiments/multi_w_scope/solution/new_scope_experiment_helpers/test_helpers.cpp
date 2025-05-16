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
	map<int, int> sum_counts;
	for (int h_index = 0; h_index < (int)this->existing_influence_indexes.size(); h_index++) {
		for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
			pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
			if (influence.second) {
				map<int, int>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, 0}).first;
				}
				it->second++;
			}
		}
	}

	map<int, int> influence_mapping;
	for (map<int, int>::iterator it = sum_counts.begin(); it != sum_counts.end(); it++) {
		if (it->second > INFLUENCE_MIN_NUM) {
			influence_mapping[it->first] = (int)influence_mapping.size();
		}
	}

	Eigen::MatrixXd inputs((int)this->existing_target_vals.size(), 1 + influence_mapping.size());
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		inputs(h_index, 0) = 1.0;
		for (int m_index = 1; m_index < 1 + (int)influence_mapping.size(); m_index++) {
			inputs(h_index, m_index) = 0.0;
		}
		for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
			pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
			if (influence.second) {
				map<int, int>::iterator it = influence_mapping.find(influence.first);
				if (it != influence_mapping.end()) {
					inputs(h_index, it->second) = 1.0;
				}
			}
		}
	}

	Eigen::VectorXd outputs((int)this->existing_target_vals.size());
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		outputs(h_index) = this->existing_target_vals[h_index];
	}

	Eigen::VectorXd weights;
	try {
		weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
	} catch (std::invalid_argument &e) {
		cout << "Eigen error" << endl;
		is_success = false;
		return;
	}

	for (int w_index = 0; w_index < 1 + (int)influence_mapping.size(); w_index++) {
		if (abs(weights(w_index)) > 10000.0) {
			is_success = false;
			return;
		}
	}

	double existing_adjust = weights(0);

	double sum_new = 0.0;
	for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
		double sum_new_influence = 0.0;
		for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
			pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
			if (influence.second) {
				map<int, int>::iterator it = influence_mapping.find(influence.first);
				if (it != influence_mapping.end()) {
					sum_new_influence += weights(it->second);
				}
			}
		}

		sum_new += this->new_target_vals[h_index] - sum_new_influence;
	}
	double new_adjust = sum_new / (int)this->new_target_vals.size();

	is_success = true;
	curr_improvement = new_adjust - existing_adjust;
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
