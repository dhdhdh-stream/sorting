#include "pass_through_experiment.h"

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
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 40;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 400;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 4000;
const int PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS = 100;
#endif /* MDEBUG */

void PassThroughExperiment::activate(AbstractNode* experiment_node,
									 bool is_branch,
									 AbstractNode*& curr_node,
									 Problem* problem,
									 RunHelper& run_helper,
									 ScopeHistory* scope_history) {
	if (this->is_branch == is_branch) {
		run_helper.num_experiment_instances++;

		map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it
			= run_helper.experiment_histories.find(this);
		PassThroughExperimentHistory* history;
		if (it == run_helper.experiment_histories.end()) {
			history = new PassThroughExperimentHistory(this);

			if (this->needs_init) {
				if (run_helper.has_explore) {
					history->is_active = false;
				} else {
					run_helper.has_explore = true;
					history->is_active = true;
				}
			} else {
				uniform_int_distribution<int> experiment_active_distribution(0, 2);
				history->is_active = experiment_active_distribution(generator) == 0;
			}

			run_helper.experiment_histories[this] = history;
		} else {
			history = (PassThroughExperimentHistory*)it->second;
		}

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
}

void PassThroughExperiment::calc_improve_helper(bool& is_success,
												double& curr_improvement) {
	{
		map<int, pair<int,int>> sum_counts;
		for (int h_index = 0; h_index < (int)this->existing_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				it->second.first++;
			}
		}
		for (int h_index = 0; h_index < (int)this->new_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
				if (it == sum_counts.end()) {
					it = sum_counts.insert({influence.first, {0,0}}).first;
				}
				it->second.second++;
			}
		}

		for (map<int, pair<int,int>>::iterator it = sum_counts.begin();
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
					is_success = false;
					return;
				}
			}
		}
	}

	double existing_sum_target_vals = 0.0;
	for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
		existing_sum_target_vals += this->existing_target_vals[h_index];
	}
	double existing_average_target_val = existing_sum_target_vals / (int)this->existing_target_vals.size();

	double existing_adjust;
	{
		map<int, pair<int,int>> sum_counts;
		for (int h_index = 0; h_index < (int)this->existing_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
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

		map<int, int> influence_mapping;
		for (map<int, pair<int,int>>::iterator it = sum_counts.begin();
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
					is_success = false;
					return;
				}

				influence_mapping[it->first] = (int)influence_mapping.size();
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
				pair<int,bool> influence = this->existing_influence_indexes[h_index][i_index];
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
			is_success = false;
			return;
		}

		if (abs(weights[0]) > 10000.0) {
			is_success = false;
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
		map<int, pair<int,int>> sum_counts;
		for (int h_index = 0; h_index < (int)this->new_influence_indexes.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
				map<int, pair<int,int>>::iterator it = sum_counts.find(influence.first);
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

		map<int, int> influence_mapping;
		for (map<int, pair<int,int>>::iterator it = sum_counts.begin();
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
					is_success = false;
					return;
				}

				influence_mapping[it->first] = (int)influence_mapping.size();
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
				pair<int,bool> influence = this->new_influence_indexes[h_index][i_index];
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
			is_success = false;
			return;
		}

		if (abs(weights[0]) > 10000.0) {
			is_success = false;
			return;
		}

		new_adjust = weights(0);
	}

	double existing_score = existing_average_target_val + existing_adjust;
	double new_score = new_average_target_val + new_adjust;

	is_success = true;
	curr_improvement = new_score - existing_score;
}

void PassThroughExperiment::backprop(double target_val,
									 bool is_return,
									 RunHelper& run_helper) {
	PassThroughExperimentHistory* history = (PassThroughExperimentHistory*)run_helper.experiment_histories[this];

	if (this->needs_init) {
		if (history->is_active) {
			if (is_return) {
				vector<pair<int,bool>> curr_influence_indexes;
				for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.experiment_histories.begin();
						it != run_helper.experiment_histories.end(); it++) {
					if (it->first != this) {
						curr_influence_indexes.push_back({it->first->multi_index, it->second->is_active});
					}
				}

				this->new_target_vals.push_back(target_val);
				this->new_influence_indexes.push_back(curr_influence_indexes);

				this->needs_init = false;
			} else {
				this->explore_iter++;
				if (this->explore_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
					this->result = EXPERIMENT_RESULT_FAIL;
				} else {
					this->step_types.clear();
					this->actions.clear();
					this->scopes.clear();

					geometric_distribution<int> geo_distribution(0.2);
					int new_num_steps = geo_distribution(generator);
					switch (this->node_context->type) {
					case NODE_TYPE_ACTION:
						{
							ActionNode* action_node = (ActionNode*)this->node_context;
							if (action_node->next_node == this->exit_next_node) {
								if (new_num_steps == 0) {
									new_num_steps = 1;
								}
							}
						}
						break;
					case NODE_TYPE_SCOPE:
						{
							ScopeNode* scope_node = (ScopeNode*)this->node_context;
							if (scope_node->next_node == this->exit_next_node) {
								if (new_num_steps == 0) {
									new_num_steps = 1;
								}
							}
						}
						break;
					case NODE_TYPE_BRANCH:
						{
							BranchNode* branch_node = (BranchNode*)this->node_context;
							if (this->is_branch) {
								if (branch_node->branch_next_node == this->exit_next_node) {
									if (new_num_steps == 0) {
										new_num_steps = 1;
									}
								}
							} else {
								if (branch_node->original_next_node == this->exit_next_node) {
									if (new_num_steps == 0) {
										new_num_steps = 1;
									}
								}
							}
						}
						break;
					case NODE_TYPE_OBS:
						{
							ObsNode* obs_node = (ObsNode*)this->node_context;
							if (obs_node->next_node == this->exit_next_node) {
								if (new_num_steps == 0) {
									new_num_steps = 1;
								}
							}
						}
						break;
					}

					/**
					 * - always give raw actions a large weight
					 *   - existing scopes often learned to avoid certain patterns
					 *     - which can prevent innovation
					 */
					uniform_int_distribution<int> scope_distribution(0, 1);
					for (int s_index = 0; s_index < new_num_steps; s_index++) {
						if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
							this->step_types.push_back(STEP_TYPE_SCOPE);
							this->actions.push_back(Action());

							uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
							this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
						} else {
							this->step_types.push_back(STEP_TYPE_ACTION);

							this->actions.push_back(problem_type->random_action());

							this->scopes.push_back(NULL);
						}
					}
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
				switch (this->state) {
				case PASS_THROUGH_EXPERIMENT_STATE_INITIAL:
					if ((int)this->new_target_vals.size() == INITIAL_NUM_SAMPLES_PER_ITER) {
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
							this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST;
						}
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_1ST:
					if ((int)this->new_target_vals.size() == VERIFY_1ST_NUM_SAMPLES_PER_ITER) {
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
							this->state = PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND;
						}
					}
					break;
				case PASS_THROUGH_EXPERIMENT_STATE_VERIFY_2ND:
					if ((int)this->new_target_vals.size() == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
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

							cout << "PassThrough" << endl;
							cout << "this->scope_context->id: " << this->scope_context->id << endl;
							cout << "this->node_context->id: " << this->node_context->id << endl;
							cout << "this->is_branch: " << this->is_branch << endl;
							cout << "new explore path:";
							for (int s_index = 0; s_index < (int)this->step_types.size(); s_index++) {
								if (this->step_types[s_index] == STEP_TYPE_ACTION) {
									cout << " " << this->actions[s_index].move;
								} else {
									cout << " E" << this->scopes[s_index]->id;
								}
							}
							cout << endl;

							cout << "this->improvement: " << this->improvement << endl;

							this->result = EXPERIMENT_RESULT_SUCCESS;
						}
					}
					break;
				}

				if (is_fail) {
					this->explore_iter++;
					if (this->explore_iter >= PASS_THROUGH_EXPERIMENT_EXPLORE_ITERS) {
						this->result = EXPERIMENT_RESULT_FAIL;
					} else {
						this->step_types.clear();
						this->actions.clear();
						this->scopes.clear();

						geometric_distribution<int> geo_distribution(0.2);
						int new_num_steps = geo_distribution(generator);
						switch (this->node_context->type) {
						case NODE_TYPE_ACTION:
							{
								ActionNode* action_node = (ActionNode*)this->node_context;
								if (action_node->next_node == this->exit_next_node) {
									if (new_num_steps == 0) {
										new_num_steps = 1;
									}
								}
							}
							break;
						case NODE_TYPE_SCOPE:
							{
								ScopeNode* scope_node = (ScopeNode*)this->node_context;
								if (scope_node->next_node == this->exit_next_node) {
									if (new_num_steps == 0) {
										new_num_steps = 1;
									}
								}
							}
							break;
						case NODE_TYPE_BRANCH:
							{
								BranchNode* branch_node = (BranchNode*)this->node_context;
								if (this->is_branch) {
									if (branch_node->branch_next_node == this->exit_next_node) {
										if (new_num_steps == 0) {
											new_num_steps = 1;
										}
									}
								} else {
									if (branch_node->original_next_node == this->exit_next_node) {
										if (new_num_steps == 0) {
											new_num_steps = 1;
										}
									}
								}
							}
							break;
						case NODE_TYPE_OBS:
							{
								ObsNode* obs_node = (ObsNode*)this->node_context;
								if (obs_node->next_node == this->exit_next_node) {
									if (new_num_steps == 0) {
										new_num_steps = 1;
									}
								}
							}
							break;
						}

						/**
						 * - always give raw actions a large weight
						 *   - existing scopes often learned to avoid certain patterns
						 *     - which can prevent innovation
						 */
						uniform_int_distribution<int> scope_distribution(0, 1);
						for (int s_index = 0; s_index < new_num_steps; s_index++) {
							if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
								this->step_types.push_back(STEP_TYPE_SCOPE);
								this->actions.push_back(Action());

								uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
								this->scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
							} else {
								this->step_types.push_back(STEP_TYPE_ACTION);

								this->actions.push_back(problem_type->random_action());

								this->scopes.push_back(NULL);
							}
						}

						this->state = PASS_THROUGH_EXPERIMENT_STATE_INITIAL;

						this->needs_init = true;
					}
				}
			} else {
				this->existing_target_vals.push_back(target_val);
				this->existing_influence_indexes.push_back(curr_influence_indexes);
			}
		}
	}
}
