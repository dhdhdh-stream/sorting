#include "multi_commit_experiment.h"

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

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INITIAL_NUM_SAMPLES_PER_ITER = 2;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 5;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 10;
const int STEP_TRY_ITERS = 2;
#else
const int INITIAL_NUM_SAMPLES_PER_ITER = 100;
const int VERIFY_1ST_NUM_SAMPLES_PER_ITER = 500;
const int VERIFY_2ND_NUM_SAMPLES_PER_ITER = 2000;
const int STEP_TRY_ITERS = 100;
#endif /* MDEBUG */

void MultiCommitExperiment::find_save_activate(
		AbstractNode*& curr_node,
		Problem* problem,
		RunHelper& run_helper,
		MultiCommitExperimentHistory* history) {
	if (history->is_active) {
		run_helper.has_explore = true;

		for (int s_index = 0; s_index < this->step_iter; s_index++) {
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

		if (this->state_iter == -1) {
			vector<AbstractNode*> possible_exits;

			AbstractNode* starting_node;
			switch (this->node_context->type) {
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					starting_node = action_node->next_node;
				}
				break;
			case NODE_TYPE_SCOPE:
				{
					ScopeNode* scope_node = (ScopeNode*)this->node_context;
					starting_node = scope_node->next_node;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						starting_node = branch_node->branch_next_node;
					} else {
						starting_node = branch_node->original_next_node;
					}
				}
				break;
			case NODE_TYPE_OBS:
				{
					ObsNode* obs_node = (ObsNode*)this->node_context;
					starting_node = obs_node->next_node;
				}
				break;
			}

			this->scope_context->random_exit_activate(
				starting_node,
				possible_exits);

			int random_index;
			if (possible_exits.size() < 20) {
				uniform_int_distribution<int> exit_distribution(0, possible_exits.size()-1);
				random_index = exit_distribution(generator);
			} else {
				geometric_distribution<int> exit_distribution(0.1);
				random_index = exit_distribution(generator);
				if (random_index >= (int)possible_exits.size()) {
					random_index = (int)possible_exits.size()-1;
				}
			}
			this->save_exit_next_node = possible_exits[random_index];

			geometric_distribution<int> geo_distribution(0.2);
			int new_num_steps = geo_distribution(generator);

			/**
			 * - always give raw actions a large weight
			 *   - existing scopes often learned to avoid certain patterns
			 *     - which can prevent innovation
			 */
			uniform_int_distribution<int> scope_distribution(0, 1);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				if (scope_distribution(generator) == 0 && this->scope_context->child_scopes.size() > 0) {
					this->save_step_types.push_back(STEP_TYPE_SCOPE);
					this->save_actions.push_back(Action());

					uniform_int_distribution<int> child_scope_distribution(0, this->scope_context->child_scopes.size()-1);
					this->save_scopes.push_back(this->scope_context->child_scopes[child_scope_distribution(generator)]);
				} else {
					this->save_step_types.push_back(STEP_TYPE_ACTION);

					this->save_actions.push_back(problem_type->random_action());

					this->save_scopes.push_back(NULL);
				}
			}

			this->state_iter = 0;
		}

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
	}
}

void MultiCommitExperiment::find_save_backprop(
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

		if ((int)this->new_target_vals.size() == INITIAL_NUM_SAMPLES_PER_ITER
				|| (int)this->new_target_vals.size() == VERIFY_1ST_NUM_SAMPLES_PER_ITER
				|| (int)this->new_target_vals.size() == VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
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
				this->existing_target_vals.clear();
				this->existing_influence_indexes.clear();
				this->new_target_vals.clear();
				this->new_influence_indexes.clear();
				this->influence_mapping.clear();

				this->save_step_types.clear();
				this->save_actions.clear();
				this->save_scopes.clear();

				this->state_iter = -1;

				// this->id = multi_index;
				// multi_index++;

				this->save_iter++;
				if (this->save_iter >= STEP_TRY_ITERS) {
					this->save_iter = 0;

					this->step_iter--;
					if (this->step_iter == 0) {
						this->result = EXPERIMENT_RESULT_FAIL;
					}
				}
			} else if ((int)this->new_target_vals.size() >= VERIFY_2ND_NUM_SAMPLES_PER_ITER) {
				this->existing_target_vals.clear();
				this->existing_influence_indexes.clear();
				this->new_target_vals.clear();
				this->new_influence_indexes.clear();
				this->influence_mapping.clear();

				// temp
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
					cout << "this->best_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->best_exit_next_node->id: " << this->best_exit_next_node->id << endl;
				}

				cout << "this->step_iter: " << this->step_iter << endl;

				cout << "new save path:";
				for (int s_index = 0; s_index < (int)this->save_step_types.size(); s_index++) {
					if (this->save_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->save_actions[s_index].move;
					} else {
						cout << " E" << this->save_scopes[s_index]->id;
					}
				}
				cout << endl;

				if (this->save_exit_next_node == NULL) {
					cout << "this->save_exit_next_node->id: " << -1 << endl;
				} else {
					cout << "this->save_exit_next_node->id: " << this->save_exit_next_node->id << endl;
				}

				cout << endl;

				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						ActionNode* new_action_node = new ActionNode();
						new_action_node->parent = this->scope_context;
						new_action_node->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;

						new_action_node->action = this->best_actions[s_index];

						new_action_node->average_instances_per_run = this->node_context->average_instances_per_run;

						this->new_nodes.push_back(new_action_node);
					} else {
						ScopeNode* new_scope_node = new ScopeNode();
						new_scope_node->parent = this->scope_context;
						new_scope_node->id = this->scope_context->node_counter;
						this->scope_context->node_counter++;

						new_scope_node->scope = this->best_scopes[s_index];

						new_scope_node->average_instances_per_run = this->node_context->average_instances_per_run;

						this->new_nodes.push_back(new_scope_node);
					}

					ObsNode* new_obs_node = new ObsNode();
					new_obs_node->parent = this->scope_context;
					new_obs_node->id = this->scope_context->node_counter;
					this->scope_context->node_counter++;

					new_obs_node->average_instances_per_run = node_context->average_instances_per_run;

					this->new_nodes.push_back(new_obs_node);
				}

				this->step_iter *= 2;

				this->state = MULTI_COMMIT_EXPERIMENT_STATE_COMMIT_EXISTING_GATHER;
				this->state_iter = 0;
			}
		}
	} else {
		this->existing_target_vals.push_back(target_val);
		this->existing_influence_indexes.push_back(curr_influence_indexes);
	}
}
