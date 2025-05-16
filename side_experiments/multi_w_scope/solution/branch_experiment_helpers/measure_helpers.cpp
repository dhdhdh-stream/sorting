#include "branch_experiment.h"

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

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 4000;
#endif /* MDEBUG */

void BranchExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		BranchExperimentHistory* history) {
	if (history->is_active) {
		if (this->select_percentage == 1.0) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->best_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
					this->best_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->exit_next_node;
		} else {
			double sum_vals = this->new_average_score;
			for (int f_index = 0; f_index < (int)this->new_factor_ids.size(); f_index++) {
				double val;
				fetch_factor_helper(scope_history,
									this->new_factor_ids[f_index],
									val);
				sum_vals += this->new_factor_weights[f_index] * val;
			}

			bool decision_is_branch;
			if (sum_vals >= 0.0) {
				decision_is_branch = true;
			} else {
				decision_is_branch = false;
			}

			if (decision_is_branch) {
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						problem->perform_action(this->best_actions[s_index]);
					} else {
						ScopeHistory* inner_scope_history = new ScopeHistory(this->best_scopes[s_index]);
						this->best_scopes[s_index]->activate(problem,
							run_helper,
							inner_scope_history);
						delete inner_scope_history;
					}

					run_helper.num_actions += 2;
				}

				curr_node = this->exit_next_node;
			}
		}
	}
}

void BranchExperiment::measure_backprop(double target_val,
										bool is_return,
										RunHelper& run_helper) {
	if (is_return) {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories[this];

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

			if ((int)this->new_target_vals.size() >= MEASURE_NUM_DATAPOINTS) {
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
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}

				for (int w_index = 0; w_index < 1 + (int)influence_mapping.size(); w_index++) {
					if (abs(weights(w_index)) > 10000.0) {
						this->result = EXPERIMENT_RESULT_FAIL;
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

				if (new_adjust > existing_adjust) {
					this->improvement = new_adjust - existing_adjust;

					cout << "BranchExperiment" << endl;
					cout << "this->scope_context->id: " << this->scope_context->id << endl;
					cout << "this->node_context->id: " << this->node_context->id << endl;
					cout << "this->is_branch: " << this->is_branch << endl;
					cout << "new explore path:";
					for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
						if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
							cout << " " << this->best_actions[s_index].move;
						} else {
							cout << " E" << this->best_scopes[s_index]->id;
						}
					}
					cout << endl;

					if (this->exit_next_node == NULL) {
						cout << "this->exit_next_node->id: " << -1 << endl;
					} else {
						cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
					}

					cout << "this->improvement: " << this->improvement << endl;

					cout << endl;

					this->result = EXPERIMENT_RESULT_SUCCESS;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		} else {
			this->existing_target_vals.push_back(target_val);
			this->existing_influence_indexes.push_back(curr_influence_indexes);
		}
	}
}
