#include "new_action_experiment.h"

#include <iostream>

#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

void NewActionExperiment::pre_activate(vector<ContextLayer>& context,
									   RunHelper& run_helper) {
	if (run_helper.experiment_histories.size() == 0) {
		bool has_seen = false;
		for (int e_index = 0; e_index < (int)run_helper.experiments_seen_order.size(); e_index++) {
			if (run_helper.experiments_seen_order[e_index] == this) {
				has_seen = true;
				break;
			}
		}
		if (!has_seen) {
			double selected_probability = 1.0 / (1.0 + this->average_remaining_experiments_from_start);
			uniform_real_distribution<double> distribution(0.0, 1.0);
			if (distribution(generator) < selected_probability) {
				NewActionExperimentHistory* history = new NewActionExperimentHistory(this);
				run_helper.experiment_histories.push_back(history);
			}

			run_helper.experiments_seen_order.push_back(this);
		}
	}
}

bool NewActionExperiment::result_activate(
		AbstractNode* experiment_node,
		bool is_branch,
		AbstractNode*& curr_node,
		Problem* problem,
		vector<ContextLayer>& context,
		RunHelper& run_helper) {
	if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories.back()->experiment == this) {
		bool has_match = false;
		int location_index;
		for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
			if (this->successful_location_starts[s_index] == experiment_node
					&& this->successful_location_is_branch[s_index] == is_branch) {
				has_match = true;
				location_index = s_index;
				break;
			}
		}

		if (has_match) {
			switch (this->state) {
			case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
				curr_node = this->successful_scope_nodes[location_index];
				return true;
			}
		}
	}

	return false;
}

bool NewActionExperiment::activate(AbstractNode* experiment_node,
								   bool is_branch,
								   AbstractNode*& curr_node,
								   Problem* problem,
								   vector<ContextLayer>& context,
								   RunHelper& run_helper) {
	if (run_helper.experiment_histories.size() == 1
			&& run_helper.experiment_histories.back()->experiment == this) {
		bool has_match = false;
		bool is_test;
		int location_index;
		for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
			if (this->test_location_starts[t_index] == experiment_node
					&& this->test_location_is_branch[t_index] == is_branch) {
				has_match = true;
				is_test = true;
				location_index = t_index;
				break;
			}
		}
		if (!has_match) {
			for (int s_index = 0; s_index < (int)this->successful_location_starts.size(); s_index++) {
				if (this->successful_location_starts[s_index] == experiment_node
						&& this->successful_location_is_branch[s_index] == is_branch) {
					has_match = true;
					is_test = false;
					location_index = s_index;
					break;
				}
			}
		}

		if (has_match) {
			NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories[0];

			switch (this->state) {
			case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
				if (is_test) {
					if (history->test_location_index == -1
							|| history->test_location_index == location_index) {
						test_activate(location_index,
									  curr_node,
									  problem,
									  context,
									  run_helper,
									  history);

						return true;
					} else {
						return false;
					}
				} else {
					curr_node = this->successful_scope_nodes[location_index];
					return true;
				}
			#if defined(MDEBUG) && MDEBUG
			case NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY:
				capture_verify_activate(location_index,
										curr_node,
										problem,
										context,
										run_helper,
										history);
				return true;
			#endif /* MDEBUG */
			}
		}
	}

	return false;
}

void NewActionExperiment::back_activate(vector<ContextLayer>& context,
										RunHelper& run_helper) {
	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories.back();

	switch (this->state) {
	case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
		if (history->test_location_index == -1) {
			uniform_int_distribution<int> select_distribution(0, history->instance_count);
			if (select_distribution(generator) == 0) {
				history->selected_nodes_seen = context.back().nodes_seen;
			}
			history->instance_count++;
		}
	}
}

void NewActionExperiment::split_activate(BranchNode* branch_node,
										 bool existing_is_branch,
										 Problem* problem,
										 vector<ContextLayer>& context,
										 RunHelper& run_helper) {
	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories.back();
	if (history->test_location_index != -1) {
		Problem* copy_problem = problem->copy_snapshot();

		RunHelper copy_run_helper = run_helper;
		copy_run_helper.is_split = true;

		vector<ContextLayer> copy_context = context;
		if (existing_is_branch) {
			copy_context.back().node = branch_node->original_next_node;
		} else {
			copy_context.back().node = branch_node->branch_next_node;
		}
		solution->scopes[0]->continue_experiment_activate(
			copy_problem,
			copy_context,
			0,
			copy_run_helper);

		double target_val;
		if (!run_helper.exceeded_limit) {
			target_val = copy_problem->score_result();
			target_val -= 0.05 * run_helper.num_actions * solution->curr_time_penalty;
			target_val -= run_helper.num_analyze * solution->curr_time_penalty;
		} else {
			target_val = -1.0;
		}

		history->existing_impacts.push_back(branch_node->impact);
		history->new_impacts.push_back(run_helper.result - target_val);

		delete copy_problem;
	}
}

void NewActionExperiment::backprop(double target_val,
								   RunHelper& run_helper) {
	NewActionExperimentHistory* history = (NewActionExperimentHistory*)run_helper.experiment_histories.back();

	switch (this->state) {
	case NEW_ACTION_EXPERIMENT_STATE_EXPLORE:
		if (history->test_location_index != -1) {
			test_backprop(target_val,
						  run_helper);
		} else {
			add_new_test_location(history);
		}

		if (this->generalize_iter >= NEW_ACTION_NUM_GENERALIZE_TRIES) {
			if (this->successful_location_starts.size() >= NEW_ACTION_MIN_LOCATIONS) {
				#if defined(MDEBUG) && MDEBUG
				for (int t_index = 0; t_index < (int)this->test_location_starts.size(); t_index++) {
					int experiment_index;
					for (int e_index = 0; e_index < (int)this->test_location_starts[t_index]->experiments.size(); e_index++) {
						if (this->test_location_starts[t_index]->experiments[e_index] == this) {
							experiment_index = e_index;
							break;
						}
					}
					this->test_location_starts[t_index]->experiments.erase(this->test_location_starts[t_index]->experiments.begin() + experiment_index);
				}
				this->test_location_starts.clear();
				this->test_location_is_branch.clear();
				this->test_location_exits.clear();
				this->test_location_states.clear();
				this->test_location_scores.clear();
				this->test_location_counts.clear();
				this->test_location_truth_counts.clear();
				for (int t_index = 0; t_index < (int)this->test_scope_nodes.size(); t_index++) {
					delete this->test_scope_nodes[t_index];
				}
				this->test_scope_nodes.clear();

				this->verify_problems = vector<Problem*>(NUM_VERIFY_SAMPLES, NULL);
				this->verify_seeds = vector<unsigned long>(NUM_VERIFY_SAMPLES);

				this->state = NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY;
				this->state_iter = 0;
				#else
				this->result = EXPERIMENT_RESULT_SUCCESS;
				#endif /* MDEBUG */
			} else {
				this->result = EXPERIMENT_RESULT_FAIL;
			}
		}

		break;
	#if defined(MDEBUG) && MDEBUG
	case NEW_ACTION_EXPERIMENT_STATE_CAPTURE_VERIFY:
		capture_verify_backprop();
		break;
	#endif /* MDEBUG */
	}
}
