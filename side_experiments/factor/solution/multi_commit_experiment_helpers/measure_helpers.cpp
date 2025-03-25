#include "multi_commit_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "action_node.h"
#include "constants.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int MEASURE_NUM_DATAPOINTS = 20;
#else
const int MEASURE_NUM_DATAPOINTS = 2000;
#endif /* MDEBUG */

void MultiCommitExperiment::measure_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history,
		MultiCommitExperimentHistory* history) {
	if (history->is_active) {
		run_helper.has_explore = true;

		for (int n_index = 0; n_index < this->step_iter; n_index++) {
			switch (this->new_nodes[n_index]->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* node = (ActionNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* node = (ObsNode*)this->new_nodes[n_index];
					node->commit_activate(problem,
										  run_helper,
										  scope_history);
				}
				break;
			}
		}

		run_helper.num_actions++;

		double sum_vals = this->commit_new_average_score;
		for (int f_index = 0; f_index < (int)this->commit_new_factor_ids.size(); f_index++) {
			double val;
			fetch_factor_helper(run_helper,
								scope_history,
								this->commit_new_factor_ids[f_index],
								val);
			sum_vals += this->commit_new_factor_weights[f_index] * val;
		}

		bool is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (run_helper.curr_run_seed%2 == 0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
		#else
		if (sum_vals >= 0.0) {
			is_branch = true;
		} else {
			is_branch = false;
		}
		#endif /* MDEBUG */

		if (is_branch) {
			for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
				if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
					problem->perform_action(this->save_actions[s_index]);
				} else {
					ScopeHistory* inner_scope_history = new ScopeHistory(this->save_scopes[s_index]);
					this->save_scopes[s_index]->activate(problem,
						run_helper,
						inner_scope_history);
					delete inner_scope_history;
				}

				run_helper.num_actions += 2;
			}

			curr_node = this->save_exit_next_node;
		} else {
			for (int n_index = this->step_iter; n_index < (int)this->new_nodes.size(); n_index++) {
				switch (this->new_nodes[n_index]->type) {
				case NODE_TYPE_ACTION:
					{
						ActionNode* node = (ActionNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				case NODE_TYPE_SCOPE:
					{
						ScopeNode* node = (ScopeNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				case NODE_TYPE_OBS:
					{
						ObsNode* node = (ObsNode*)this->new_nodes[n_index];
						node->commit_activate(problem,
											  run_helper,
											  scope_history);
					}
					break;
				}
			}

			curr_node = this->best_exit_next_node;
		}
	}
}

void MultiCommitExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper,
		MultiCommitExperimentHistory* history) {
	vector<int> curr_influence_indexes;
	for (map<AbstractExperiment*, AbstractExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
			it != run_helper.multi_experiment_histories.end(); it++) {
		MultiCommitExperiment* multi_commit_experiment = (MultiCommitExperiment*)it->first;
		MultiCommitExperimentHistory* multi_commit_experiment_history
			= (MultiCommitExperimentHistory*)it->second;
		if (multi_commit_experiment != this && multi_commit_experiment_history->is_active) {
			int index;
			map<int, int>::iterator m_it = this->influence_mapping.find(multi_commit_experiment->id);
			if (m_it == this->influence_mapping.end()) {
				index = 1 + (int)this->influence_mapping.size();
				this->influence_mapping[multi_commit_experiment->id] = 1 + (int)this->influence_mapping.size();
			} else {
				index = m_it->second;
			}
			curr_influence_indexes.push_back(index);
		}
	}

	if (history->is_active) {
		this->new_target_vals.push_back(target_val);
		this->new_influence_indexes.push_back(curr_influence_indexes);
	} else {
		this->existing_target_vals.push_back(target_val);
		this->existing_influence_indexes.push_back(curr_influence_indexes);
	}

	if ((int)this->new_target_vals.size() >= MEASURE_NUM_DATAPOINTS) {
		int num_instances = (int)this->existing_target_vals.size() + (int)this->new_target_vals.size();

		double sum_target_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			sum_target_vals += this->existing_target_vals[h_index];
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			sum_target_vals += this->new_target_vals[h_index];
		}
		double average_target_val = sum_target_vals / num_instances;

		Eigen::MatrixXd inputs(num_instances, 1 + this->influence_mapping.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int m_index = 0; m_index < 1 + (int)this->influence_mapping.size(); m_index++) {
				inputs(i_index, m_index) = 0.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				inputs(h_index, this->existing_influence_indexes[h_index][i_index]) = 1.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			inputs((int)this->existing_target_vals.size() + h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				inputs((int)this->existing_target_vals.size() + h_index, this->new_influence_indexes[h_index][i_index]) = 1.0;
			}
		}

		Eigen::VectorXd outputs(num_instances);
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			outputs(h_index) = this->existing_target_vals[h_index] - average_target_val;
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			outputs((int)this->existing_target_vals.size() + h_index) = this->new_target_vals[h_index] - average_target_val;
		}

		Eigen::VectorXd weights;
		try {
			weights = inputs.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(outputs);
		} catch (std::invalid_argument &e) {
			cout << "Eigen error" << endl;
			this->result = EXPERIMENT_RESULT_FAIL;
			return;
		}

		if (abs(weights(0)) > 10000.0) {
			this->result = EXPERIMENT_RESULT_FAIL;
			return;
		}

		if (weights(0) < 0.0) {
			this->result = EXPERIMENT_RESULT_SUCCESS;

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
			if (this->best_exit_next_node == NULL) {
				cout << "this->best_exit_next_node->id: -1" << endl;
			} else {
				cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
			}
			cout << endl;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	}
}
