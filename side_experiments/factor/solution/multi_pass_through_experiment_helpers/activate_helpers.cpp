#include "multi_pass_through_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

#include "abstract_node.h"
#include "constants.h"
#include "problem.h"
#include "scope.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
#endif /* MDEBUG */

void MultiPassThroughExperiment::activate(
		AbstractNode* experiment_node,
		bool is_branch,
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		ScopeHistory* scope_history) {
	MultiPassThroughExperimentHistory* history;
	map<MultiPassThroughExperiment*, MultiPassThroughExperimentHistory*>::iterator it
		= run_helper.multi_experiment_histories.find(this);
	if (it == run_helper.multi_experiment_histories.end()) {
		history = new MultiPassThroughExperimentHistory(this);
		run_helper.multi_experiment_histories[this] = history;
	} else {
		history = it->second;
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

void MultiPassThroughExperiment::backprop(
		double target_val,
		RunHelper& run_helper) {
	MultiPassThroughExperimentHistory* history = run_helper.multi_experiment_histories[this];

	vector<int> curr_influence_indexes;
	for (map<MultiPassThroughExperiment*, MultiPassThroughExperimentHistory*>::iterator it = run_helper.multi_experiment_histories.begin();
			it != run_helper.multi_experiment_histories.end(); it++) {
		if (it->first != this && it->second->is_active) {
			int index;
			map<int, int>::iterator m_it = this->influence_mapping.find(it->first->id);
			if (m_it == this->influence_mapping.end()) {
				index = 1 + (int)this->influence_mapping.size();
				this->influence_mapping[it->first->id] = 1 + (int)this->influence_mapping.size();
			} else {
				index = m_it->second;
			}
			curr_influence_indexes.push_back(index);
		}
	}

	if (history->is_active) {
		this->new_target_vals.push_back(target_val);
		this->new_influence_indexes.push_back(curr_influence_indexes);

		if ((int)this->new_target_vals.size() == INITIAL_NUM_SAMPLES_PER_ITER
				|| (int)this->new_target_vals.size() == VERIFY_1ST_NUM_SAMPLES_PER_ITER
				|| (int)this->new_target_vals.size() == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			double existing_sum_target_vals = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
				existing_sum_target_vals += this->existing_target_vals[h_index];
			}
			double existing_average_target_val = existing_sum_target_vals / (int)this->existing_target_vals.size();

			double existing_adjust;
			{
				Eigen::MatrixXd inputs((int)this->existing_target_vals.size(), 1 + this->influence_mapping.size());
				for (int i_index = 0; i_index < (int)this->existing_target_vals.size(); i_index++) {
					for (int m_index = 0; m_index < 1 + (int)this->influence_mapping.size(); m_index++) {
						inputs(i_index, m_index) = 0.0;
					}
				}
				for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
					inputs(h_index, 0) = 1.0;
					for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
						inputs(h_index, this->existing_influence_indexes[h_index][i_index]) = 1.0;
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
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}

				if (abs(weights[0]) > 10000.0) {
					this->result = EXPERIMENT_RESULT_FAIL;
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
				Eigen::MatrixXd inputs((int)this->new_target_vals.size(), 1 + this->influence_mapping.size());
				for (int i_index = 0; i_index < (int)this->new_target_vals.size(); i_index++) {
					for (int m_index = 0; m_index < 1 + (int)this->influence_mapping.size(); m_index++) {
						inputs(i_index, m_index) = 0.0;
					}
				}
				for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
					inputs(h_index, 0) = 1.0;
					for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
						inputs(h_index, this->new_influence_indexes[h_index][i_index]) = 1.0;
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
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}

				if (abs(weights[0]) > 10000.0) {
					this->result = EXPERIMENT_RESULT_FAIL;
					return;
				}

				new_adjust = weights(0);
			}

			double existing_score = existing_average_target_val + existing_adjust;
			double new_score = new_average_target_val + new_adjust;

			if (new_score <= existing_score) {
				this->result = EXPERIMENT_RESULT_FAIL;
				return;
			}
		}

		if ((int)this->new_target_vals.size() >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
			this->result = EXPERIMENT_RESULT_SUCCESS;

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
			if (this->exit_next_node == NULL) {
				cout << "this->exit_next_node->id: -1" << endl;
			} else {
				cout << "this->exit_next_node->id: " << this->exit_next_node->id << endl;
			}
			cout << endl;
		}
	} else {
		this->existing_target_vals.push_back(target_val);
		this->existing_influence_indexes.push_back(curr_influence_indexes);
	}
}
