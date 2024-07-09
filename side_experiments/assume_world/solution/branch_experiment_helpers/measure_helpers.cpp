#include "branch_experiment.h"

#include <cmath>
#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "minesweeper.h"
#include "network.h"
#include "scope_node.h"
#include "solution_set.h"
#include "utilities.h"

using namespace std;

bool BranchExperiment::measure_activate(AbstractNode*& curr_node,
										Problem* problem,
										vector<ContextLayer>& context,
										RunHelper& run_helper,
										BranchExperimentHistory* history) {
	if (run_helper.branch_node_ancestors.find(this->branch_node) != run_helper.branch_node_ancestors.end()) {
		return false;
	}

	run_helper.branch_node_ancestors.insert(this->branch_node);

	history->instance_count++;

	run_helper.num_analyze += (1 + 2*this->analyze_size) * (1 + 2*this->analyze_size);

	vector<vector<double>> input_vals(1 + 2*this->analyze_size);
	for (int x_index = 0; x_index < 1 + 2*this->analyze_size; x_index++) {
		input_vals[x_index] = vector<double>(1 + 2*this->analyze_size);
	}

	Minesweeper* minesweeper = (Minesweeper*)problem;
	for (int x_index = -this->analyze_size; x_index < this->analyze_size+1; x_index++) {
		for (int y_index = -this->analyze_size; y_index < this->analyze_size+1; y_index++) {
			input_vals[x_index + this->analyze_size][y_index + this->analyze_size]
				= minesweeper->get_observation_helper(
					minesweeper->current_x + x_index,
					minesweeper->current_y + y_index);
		}
	}
	this->new_network->activate(input_vals);

	#if defined(MDEBUG) && MDEBUG
	bool decision_is_branch;
	if (run_helper.curr_run_seed%2 == 0) {
		decision_is_branch = true;
	} else {
		decision_is_branch = false;
	}
	run_helper.curr_run_seed = xorshift(run_helper.curr_run_seed);
	#else
	bool decision_is_branch = this->new_network->output->acti_vals[0] >= 0.0;
	#endif /* MDEBUG */

	run_helper.num_actions++;
	context.back().nodes_seen.push_back({this->branch_node, decision_is_branch});

	if (decision_is_branch) {
		this->branch_count++;

		if (this->best_step_types.size() == 0) {
			curr_node = this->best_exit_next_node;
		} else {
			if (this->best_step_types[0] == STEP_TYPE_ACTION) {
				curr_node = this->best_actions[0];
			} else {
				curr_node = this->best_scopes[0];
			}
		}

		return true;
	} else {
		this->original_count++;

		return false;
	}
}

void BranchExperiment::measure_backprop(
		double target_val,
		RunHelper& run_helper) {
	if (run_helper.exceeded_limit) {
		this->explore_iter++;
		if (this->explore_iter < MAX_EXPLORE_TRIES) {
			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_scopes[s_index];
				}
			}

			this->best_step_types.clear();
			this->best_actions.clear();
			this->best_scopes.clear();

			delete this->branch_node;
			this->branch_node = NULL;
			if (this->ending_node != NULL) {
				delete this->ending_node;
				this->ending_node = NULL;
			}

			delete this->new_network;
			this->new_network = NULL;

			uniform_int_distribution<int> neutral_distribution(0, 9);
			if (neutral_distribution(generator) == 0) {
				this->explore_type = EXPLORE_TYPE_NEUTRAL;
			} else {
				uniform_int_distribution<int> best_distribution(0, 1);
				if (best_distribution(generator) == 0) {
					this->explore_type = EXPLORE_TYPE_BEST;

					this->best_surprise = 0.0;
				} else {
					this->explore_type = EXPLORE_TYPE_GOOD;
				}
			}

			this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
			this->state_iter = 0;
		} else {
			this->result = EXPERIMENT_RESULT_FAIL;
		}
	} else {
		BranchExperimentHistory* history = (BranchExperimentHistory*)run_helper.experiment_histories.back();

		for (int i_index = 0; i_index < history->instance_count; i_index++) {
			double final_score = (target_val - solution_set->average_score) / history->instance_count;
			this->combined_score += final_score;
			this->sub_state_iter++;
		}

		this->state_iter++;
		if (this->sub_state_iter >= NUM_DATAPOINTS
				&& this->state_iter >= MIN_NUM_TRUTH_DATAPOINTS) {
			this->combined_score /= this->sub_state_iter;

			this->branch_weight = (double)this->branch_count / (double)(this->original_count + this->branch_count);

			#if defined(MDEBUG) && MDEBUG
			if (rand()%2 == 0) {
			#else
			if (this->branch_weight > 0.01
					&& this->combined_score > this->existing_average_score) {
			#endif /* MDEBUG */
				#if defined(MDEBUG) && MDEBUG
				if (rand()%4 == 0) {
				#else
				if (this->branch_weight > PASS_THROUGH_BRANCH_WEIGHT
						&& this->new_average_score > 0.0) {
				#endif
					this->is_pass_through = true;
				} else {
					this->is_pass_through = false;
				}

				this->target_val_histories.reserve(VERIFY_NUM_DATAPOINTS);

				this->state = BRANCH_EXPERIMENT_STATE_VERIFY_EXISTING;
				this->state_iter = 0;
			} else {
				this->explore_iter++;
				if (this->explore_iter < MAX_EXPLORE_TRIES) {
					for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
						if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
							delete this->best_actions[s_index];
						} else {
							delete this->best_scopes[s_index];
						}
					}

					this->best_step_types.clear();
					this->best_actions.clear();
					this->best_scopes.clear();

					delete this->branch_node;
					this->branch_node = NULL;
					if (this->ending_node != NULL) {
						delete this->ending_node;
						this->ending_node = NULL;
					}

					delete this->new_network;
					this->new_network = NULL;

					uniform_int_distribution<int> neutral_distribution(0, 9);
					if (neutral_distribution(generator) == 0) {
						this->explore_type = EXPLORE_TYPE_NEUTRAL;
					} else {
						uniform_int_distribution<int> best_distribution(0, 1);
						if (best_distribution(generator) == 0) {
							this->explore_type = EXPLORE_TYPE_BEST;

							this->best_surprise = 0.0;
						} else {
							this->explore_type = EXPLORE_TYPE_GOOD;
						}
					}

					this->state = BRANCH_EXPERIMENT_STATE_EXPLORE;
					this->state_iter = 0;
				} else {
					this->result = EXPERIMENT_RESULT_FAIL;
				}
			}
		}
	}
}
