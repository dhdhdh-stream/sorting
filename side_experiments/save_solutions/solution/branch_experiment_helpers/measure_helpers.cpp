#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "helpers.h"
#include "network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"
#include "utilities.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int SIGNAL_CHECK_MIN_NUM = 5;
#else
const int SIGNAL_CHECK_MIN_NUM = 200;
#endif /* MDEBUG */

void BranchExperiment::measure_check_activate(SolutionWrapper* wrapper) {
	ScopeHistory* scope_history = wrapper->scope_histories.back();

	if (this->select_percentage == 1.0) {
		BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
		new_experiment_state->step_index = 0;
		wrapper->experiment_context.back() = new_experiment_state;
	} else {
		double sum_vals = this->new_average_score;

		for (int i_index = 0; i_index < (int)this->new_inputs.size(); i_index++) {
			double val;
			bool is_on;
			fetch_input_helper(scope_history,
							   this->new_inputs[i_index],
							   0,
							   val,
							   is_on);
			if (is_on) {
				double normalized_val = (val - this->new_input_averages[i_index]) / this->new_input_standard_deviations[i_index];
				sum_vals += this->new_weights[i_index] * normalized_val;
			}
		}

		if (this->new_network != NULL) {
			vector<double> input_vals(this->new_network_inputs.size());
			vector<bool> input_is_on(this->new_network_inputs.size());
			for (int i_index = 0; i_index < (int)this->new_network_inputs.size(); i_index++) {
				double val;
				bool is_on;
				fetch_input_helper(scope_history,
								   this->new_network_inputs[i_index],
								   0,
								   val,
								   is_on);
				input_vals[i_index] = val;
				input_is_on[i_index] = is_on;
			}
			this->new_network->activate(input_vals,
										input_is_on);
			sum_vals += this->new_network->output->acti_vals[0];
		}

		bool decision_is_branch;
		#if defined(MDEBUG) && MDEBUG
		if (wrapper->curr_run_seed%2 == 0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		wrapper->curr_run_seed = xorshift(wrapper->curr_run_seed);
		#else
		if (sum_vals >= 0.0) {
			decision_is_branch = true;
		} else {
			decision_is_branch = false;
		}
		#endif /* MDEBUG */

		BranchNodeHistory* branch_node_history = new BranchNodeHistory(this->new_branch_node);
		branch_node_history->index = (int)scope_history->node_histories.size();
		scope_history->node_histories[this->new_branch_node->id] = branch_node_history;

		branch_node_history->is_branch = decision_is_branch;

		if (decision_is_branch) {
			BranchExperimentState* new_experiment_state = new BranchExperimentState(this);
			new_experiment_state->step_index = 0;
			wrapper->experiment_context.back() = new_experiment_state;
		}
	}
}

void BranchExperiment::measure_step(vector<double>& obs,
									int& action,
									bool& is_next,
									SolutionWrapper* wrapper,
									BranchExperimentState* experiment_state) {
	if (experiment_state->step_index >= (int)this->new_nodes.size()) {
		wrapper->node_context.back() = this->best_exit_next_node;

		delete experiment_state;
		wrapper->experiment_context.back() = NULL;
	} else {
		ScopeHistory* scope_history = wrapper->scope_histories.back();

		switch (this->new_nodes[experiment_state->step_index]->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* node = (ActionNode*)this->new_nodes[experiment_state->step_index];

				ActionNodeHistory* history = new ActionNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				action = node->action;
				is_next = true;

				wrapper->num_actions++;

				experiment_state->step_index++;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* node = (ScopeNode*)this->new_nodes[experiment_state->step_index];

				ScopeNodeHistory* history = new ScopeNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				ScopeHistory* inner_scope_history = new ScopeHistory(node->scope);
				history->scope_history = inner_scope_history;
				wrapper->scope_histories.push_back(inner_scope_history);
				wrapper->node_context.push_back(node->scope->nodes[0]);
				wrapper->experiment_context.push_back(NULL);
			}
			break;
		case NODE_TYPE_OBS:
			{
				ObsNode* node = (ObsNode*)this->new_nodes[experiment_state->step_index];

				ObsNodeHistory* history = new ObsNodeHistory(node);
				history->index = (int)scope_history->node_histories.size();
				scope_history->node_histories[node->id] = history;

				history->obs_history = obs;

				experiment_state->step_index++;
			}
			break;
		}
	}
}

void BranchExperiment::measure_exit_step(SolutionWrapper* wrapper,
										 BranchExperimentState* experiment_state) {
	wrapper->scope_histories.pop_back();
	wrapper->node_context.pop_back();
	wrapper->experiment_context.pop_back();

	experiment_state->step_index++;
}

void BranchExperiment::measure_backprop(double target_val,
										SolutionWrapper* wrapper) {
	BranchExperimentOverallHistory* overall_history = (BranchExperimentOverallHistory*)wrapper->experiment_overall_history;

	if (overall_history->is_hit) {
		this->new_scores.push_back(target_val);

		vector<ScopeHistory*> scope_histories;
		fetch_signals_helper(wrapper->scope_histories[0],
							 scope_histories,
							 this->scope_context,
							 this->node_context,
							 this->is_branch,
							 this->new_signals,
							 wrapper);

		this->state_iter++;
		if (this->state_iter >= MEASURE_ITERS) {
			double existing_sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_score += this->existing_scores[h_index];
			}
			double existing_score_average = existing_sum_score / (double)this->existing_scores.size();

			double existing_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->existing_scores.size(); h_index++) {
				existing_sum_variance += (this->existing_scores[h_index] - existing_score_average)
					* (this->existing_scores[h_index] - existing_score_average);
			}
			double existing_score_standard_deviation = sqrt(existing_sum_variance / (double)this->existing_scores.size());

			double new_sum_score = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_score += this->new_scores[h_index];
			}
			double new_score_average = new_sum_score / (double)this->new_scores.size();

			double new_sum_variance = 0.0;
			for (int h_index = 0; h_index < (int)this->new_scores.size(); h_index++) {
				new_sum_variance += (this->new_scores[h_index] - new_score_average)
					* (this->new_scores[h_index] - new_score_average);
			}
			double new_score_standard_deviation = sqrt(new_sum_variance / (double)this->new_scores.size());

			double score_improvement = new_score_average - existing_score_average;
			double existing_score_standard_error = existing_score_standard_deviation / sqrt((double)this->existing_scores.size());
			double new_score_standard_error = new_score_standard_deviation / sqrt((double)this->new_scores.size());
			double score_t_score = score_improvement / sqrt(
				existing_score_standard_error * existing_score_standard_error
					+ new_score_standard_error * new_score_standard_error);

			cout << "existing_score_average: " << existing_score_average << endl;
			cout << "new_score_average: " << new_score_average << endl;
			cout << "score_t_score: " << score_t_score << endl;

			#if defined(MDEBUG) && MDEBUG
			if (new_score_average <= existing_score_average && rand()%2 == 0) {
			#else
			if (new_score_average <= existing_score_average) {
			#endif /* MDEBUG */
				/**
				 * TODO: add per scope
				 */
				bool add_to_trap = false;
				if (score_t_score < -2.326) {
					for (map<Scope*, vector<double>>::iterator existing_it = existing_signals.begin();
							existing_it != existing_signals.end(); existing_it++) {
						if (existing_it->second.size() >= SIGNAL_CHECK_MIN_NUM) {
							map<Scope*, vector<double>>::iterator new_it = new_signals.find(existing_it->first);
							if (new_it != new_signals.end() && new_it->second.size() >= SIGNAL_CHECK_MIN_NUM) {
								double existing_sum_vals = 0.0;
								for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
									existing_sum_vals += existing_it->second[h_index];
								}
								double existing_average = existing_sum_vals / (double)existing_it->second.size();

								double new_sum_vals = 0.0;
								for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
									new_sum_vals += new_it->second[h_index];
								}
								double new_average = new_sum_vals / (double)new_it->second.size();

								cout << "existing_average: " << existing_average << endl;
								cout << "new_average: " << new_average << endl;

								if (new_average > existing_average) {
									add_to_trap = true;
									break;
								}
							}
						}
					}
				} else if (score_t_score < 0.0) {
					for (map<Scope*, vector<double>>::iterator existing_it = existing_signals.begin();
							existing_it != existing_signals.end(); existing_it++) {
						if (existing_it->second.size() >= SIGNAL_CHECK_MIN_NUM) {
							map<Scope*, vector<double>>::iterator new_it = new_signals.find(existing_it->first);
							if (new_it != new_signals.end() && new_it->second.size() >= SIGNAL_CHECK_MIN_NUM) {
								double existing_sum_vals = 0.0;
								for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
									existing_sum_vals += existing_it->second[h_index];
								}
								double existing_average = existing_sum_vals / (double)existing_it->second.size();

								double existing_sum_variance = 0.0;
								for (int h_index = 0; h_index < (int)existing_it->second.size(); h_index++) {
									existing_sum_variance += (existing_it->second[h_index] - existing_average)
										* (existing_it->second[h_index] - existing_average);
								}
								double existing_standard_deviation = sqrt(existing_sum_variance / (double)existing_it->second.size());
								double existing_denom = existing_standard_deviation / sqrt((double)existing_it->second.size());

								double new_sum_vals = 0.0;
								for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
									new_sum_vals += new_it->second[h_index];
								}
								double new_average = new_sum_vals / (double)new_it->second.size();

								double new_sum_variance = 0.0;
								for (int h_index = 0; h_index < (int)new_it->second.size(); h_index++) {
									new_sum_variance += (new_it->second[h_index] - new_average)
										* (new_it->second[h_index] - new_average);
								}
								double new_standard_deviation = sqrt(new_sum_variance / (double)new_it->second.size());
								double new_denom = new_standard_deviation / sqrt((double)new_it->second.size());

								double diff = new_average - existing_average;
								double t_score = diff / sqrt(existing_denom * existing_denom
									+ new_denom * new_denom);

								cout << "existing_average: " << existing_average << endl;
								cout << "new_average: " << new_average << endl;
								cout << "t_score: " << t_score << endl;

								if (t_score > 2.326) {
									add_to_trap = true;
									break;
								}
							}
						}
					}
				}
				if (add_to_trap) {
					Solution* solution_copy = new Solution(wrapper->solution);
					/**
					 * - TODO: move scope_histories over
					 */

					add(wrapper);

					for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
						delete wrapper->solution->existing_scope_histories[h_index];
					}
					wrapper->solution->existing_scope_histories.clear();
					wrapper->solution->existing_target_val_histories.clear();

					wrapper->solution->existing_scope_histories = this->new_scope_histories;
					this->new_scope_histories.clear();
					wrapper->solution->existing_target_val_histories = this->new_target_val_histories;
					clean_scope(this->scope_context,
								wrapper);

					wrapper->solution->clean();

					wrapper->solution->measure_update();

					wrapper->trap_solutions.push_back(wrapper->solution);
					wrapper->solution = solution_copy;
				}

				this->result = EXPERIMENT_RESULT_FAIL;
			} else {
				this->improvement = new_score_average - existing_score_average;

				cout << "BranchExperiment" << endl;
				cout << "this->scope_context->id: " << this->scope_context->id << endl;
				cout << "this->node_context->id: " << this->node_context->id << endl;
				cout << "this->is_branch: " << this->is_branch << endl;
				cout << "new explore path:";
				for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
					if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
						cout << " " << this->best_actions[s_index];
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

				cout << "this->select_percentage: " << this->select_percentage << endl;

				cout << "this->improvement: " << this->improvement << endl;

				cout << endl;

				#if defined(MDEBUG) && MDEBUG
				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = BRANCH_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				Solution* solution_copy = new Solution(wrapper->solution);
				/**
				 * - TODO: move scope_histories over
				 */

				add(wrapper);

				for (int h_index = 0; h_index < (int)wrapper->solution->existing_scope_histories.size(); h_index++) {
					delete wrapper->solution->existing_scope_histories[h_index];
				}
				wrapper->solution->existing_scope_histories.clear();
				wrapper->solution->existing_target_val_histories.clear();

				wrapper->solution->existing_scope_histories = this->new_scope_histories;
				this->new_scope_histories.clear();
				wrapper->solution->existing_target_val_histories = this->new_target_val_histories;
				clean_scope(this->scope_context,
							wrapper);

				wrapper->solution->clean();

				wrapper->solution->measure_update();

				this->resulting_solution = wrapper->solution;
				wrapper->solution = solution_copy;

				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			}
		}
	}
}
