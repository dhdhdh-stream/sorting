#include "multi_pass_through_experiment.h"

#include <cmath>
#include <iostream>
#undef eigen_assert
#define eigen_assert(x) if (!(x)) {throw std::invalid_argument("Eigen error");}
#include <Eigen/Dense>

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
		int index;
		map<int, int>::iterator m_it = this->influence_mapping.find(it->first->id);
		if (m_it == this->influence_mapping.end()) {
			index = 2 + (int)this->influence_mapping.size();
			this->influence_mapping[it->first->id] = 2 + (int)this->influence_mapping.size();
		} else {
			index = m_it->second;
		}
		curr_influence_indexes.push_back(index);
	}

	if (history->is_active) {
		this->new_target_vals.push_back(target_val);
		this->new_influence_indexes.push_back(curr_influence_indexes);
	} else {
		this->existing_target_vals.push_back(target_val);
		this->existing_influence_indexes.push_back(curr_influence_indexes);
	}

	if ((int)this->new_target_vals.size() == INITIAL_NUM_SAMPLES_PER_ITER
			|| (int)this->new_target_vals.size() == VERIFY_1ST_NUM_SAMPLES_PER_ITER
			|| (int)this->new_target_vals.size() == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
		int num_instances = (int)this->existing_target_vals.size() + (int)this->new_target_vals.size();
		Eigen::MatrixXd inputs(num_instances, 2 + this->influence_mapping.size());
		for (int i_index = 0; i_index < num_instances; i_index++) {
			for (int m_index = 0; m_index < 2 + (int)this->influence_mapping.size(); m_index++) {
				inputs(i_index, m_index) = 0.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			inputs(h_index, 0) = 1.0;
			for (int i_index = 0; i_index < (int)this->existing_influence_indexes[h_index].size(); i_index++) {
				inputs(h_index, this->existing_influence_indexes[h_index][i_index]) = 1.0;
			}
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			inputs((int)this->existing_target_vals.size() + h_index, 1) = 1.0;
			for (int i_index = 0; i_index < (int)this->new_influence_indexes[h_index].size(); i_index++) {
				inputs((int)this->existing_target_vals.size() + h_index, this->new_influence_indexes[h_index][i_index]) = 1.0;
			}
		}

		double sum_target_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->existing_target_vals.size(); h_index++) {
			sum_target_vals += this->existing_target_vals[h_index];
		}
		for (int h_index = 0; h_index < (int)this->new_target_vals.size(); h_index++) {
			sum_target_vals += this->new_target_vals[h_index];
		}
		double average_target_val = sum_target_vals / num_instances;

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

		if (weights(1) <= weights(0)) {
			this->result = EXPERIMENT_RESULT_FAIL;
			return;
		}
	}

	if ((int)this->new_target_vals.size() >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
		this->result = EXPERIMENT_RESULT_SUCCESS;
	}
}
