#include "experiment.h"

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "branch_network.h"
#include "experiment_run.h"
#include "globals.h"
#include "solution.h"
#include "start_node.h"
#include "world_model_helpers.h"
#include "wrapper.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int GATHER_NUM_SAMPLES = 40;
#else
const int GATHER_NUM_SAMPLES = 4000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int TRAIN_ITERS = 30;
#else
const int TRAIN_ITERS = 300000;
#endif /* MDEBUG */

#if defined(MDEBUG) && MDEBUG
const int NUM_EXPLORE = 5;
#else
const int NUM_EXPLORE = 1000;
#endif /* MDEBUG */

void Experiment::gather_activate(ExperimentRun* run) {
	map<Experiment*, ExperimentHistory*>::iterator it =
		run->experiment_histories.find(this);
	if (it == run->experiment_histories.end()) {
		run->experiment_histories[this] = new ExperimentHistory(this);
	}

	this->start_state_history.push_back(run->state);
}

void Experiment::gather_exit(ExperimentRun* run) {
	map<Experiment*, ExperimentHistory*>::iterator it =
		run->experiment_histories.find(this);
	if (it == run->experiment_histories.end()) {
		run->experiment_histories[this] = new ExperimentHistory(this);
	}

	this->end_state_history.push_back(run->state);
}

void Experiment::gather_backprop(double target_val,
								 ExperimentHistory* history,
								 Wrapper* wrapper) {
	if (this->start_target_val_history.size() < this->start_state_history.size()) {
		this->start_target_val_history.push_back(target_val);
		this->start_hits++;
	}
	if (this->end_target_val_history.size() < this->end_state_history.size()) {
		this->end_target_val_history.push_back(target_val);
		this->end_hits++;
	}

	if (this->start_hits >= GATHER_NUM_SAMPLES
			&& this->end_hits >= GATHER_NUM_SAMPLES) {
		for (int e_index = 0; e_index < (int)this->exit_next_node->exit_experiments.size(); e_index++) {
			if (this->exit_next_node->exit_experiments[e_index] == this) {
				this->exit_next_node->exit_experiments.erase(this->exit_next_node->exit_experiments.begin() + e_index);
				break;
			}
		}

		uniform_int_distribution<int> start_distribution(0, this->start_state_history.size()-1);
		uniform_int_distribution<int> end_distribution(0, this->end_state_history.size()-1);

		this->original_network = new BranchNetwork(this->start_state_history[0].size());
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = start_distribution(generator);
			this->original_network->activate(this->start_state_history[index]);
			double error = this->start_target_val_history[index] - this->original_network->output->acti_vals[0];
			this->original_network->backprop(error);
		}

		BranchNetwork* end_network = new BranchNetwork(this->end_state_history[0].size());
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = end_distribution(generator);
			end_network->activate(this->end_state_history[index]);
			double error = this->end_target_val_history[index] - end_network->output->acti_vals[0];
			end_network->backprop(error);
		}

		bool exit_in_place;
		switch (this->node_context->type) {
		case NODE_TYPE_START:
			{
				StartNode* start_node = (StartNode*)this->node_context;
				if (this->exit_next_node == start_node->next_node) {
					exit_in_place = true;
				} else {
					exit_in_place = false;
				}
			}
			break;
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)this->node_context;
				if (this->exit_next_node == action_node->next_node) {
					exit_in_place = true;
				} else {
					exit_in_place = false;
				}
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)this->node_context;
				if (this->is_branch) {
					if (this->exit_next_node == branch_node->branch_next_node) {
						exit_in_place = true;
					} else {
						exit_in_place = false;
					}
				} else {
					if (this->exit_next_node == branch_node->original_next_node) {
						exit_in_place = true;
					} else {
						exit_in_place = false;
					}
				}
			}
			break;
		}

		vector<int> best_actions;
		double best_surprise = numeric_limits<double>::lowest();
		for (int e_index = 0; e_index < NUM_EXPLORE; e_index++) {
			vector<double> state = this->start_state_history[start_distribution(generator)];
			this->original_network->activate(state);
			double existing_predicted = this->original_network->output->acti_vals[0];

			int new_num_steps;
			geometric_distribution<int> geo_distribution(0.3);
			if (exit_in_place) {
				new_num_steps = 1 + geo_distribution(generator);
			} else {
				new_num_steps = geo_distribution(generator);
			}

			vector<int> curr_actions;
			uniform_int_distribution<int> action_distribution(0, 3);
			for (int s_index = 0; s_index < new_num_steps; s_index++) {
				curr_actions.push_back(action_distribution(generator));
			}

			for (int a_index = 0; a_index < (int)curr_actions.size(); a_index++) {
				predict_helper(curr_actions[a_index],
							   state,
							   wrapper);
			}

			end_network->activate(state);
			double predicted = end_network->output->acti_vals[0];

			double curr_surprise = predicted - existing_predicted;
			if (curr_surprise > best_surprise) {
				best_actions = curr_actions;
				best_surprise = curr_surprise;
			}
		}

		vector<double> new_predicted(this->start_state_history.size());
		for (int h_index = 0; h_index < (int)this->start_state_history.size(); h_index++) {
			vector<double> state = this->start_state_history[h_index];
			for (int a_index = 0; a_index < (int)best_actions.size(); a_index++) {
				predict_helper(best_actions[a_index],
							   state,
							   wrapper);
			}
			end_network->activate(state);
			new_predicted[h_index] = end_network->output->acti_vals[0];
		}

		delete end_network;

		this->branch_network = new BranchNetwork(this->start_state_history.size());
		for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
			int index = start_distribution(generator);
			this->branch_network->activate(this->start_state_history[index]);
			double error = new_predicted[index] - this->branch_network->output->acti_vals[0];
			this->branch_network->backprop(error);
		}

		double sum_vals = 0.0;
		for (int h_index = 0; h_index < (int)this->start_state_history.size(); h_index++) {
			this->original_network->activate(this->start_state_history[h_index]);
			this->branch_network->activate(this->start_state_history[h_index]);
			if (this->branch_network->output->acti_vals[0] > this->original_network->output->acti_vals[0]) {
				sum_vals += new_predicted[h_index] - this->original_network->output->acti_vals[0];
			}
		}
		double local_improvement = sum_vals / (double)this->start_state_history.size();

		int total_iters = wrapper->iter - this->starting_iter;
		if (total_iters < 0) {
			total_iters += numeric_limits<int>::max();
		}
		double average_instances_per_run = (double)this->start_state_history.size() / (double)total_iters;

		double global_improvement = average_instances_per_run * local_improvement;

		bool is_success = false;
		if (local_improvement > 0.0) {
			if (wrapper->solution->train_new_last_scores.size() >= MIN_NUM_LAST_TRACK) {
				int num_better_than = 0;
				for (list<double>::iterator it = wrapper->solution->train_new_last_scores.begin();
						it != wrapper->solution->train_new_last_scores.end(); it++) {
					if (global_improvement >= *it) {
						num_better_than++;
					}
				}

				double target_better_than = LAST_BETTER_THAN_RATIO * (double)wrapper->solution->train_new_last_scores.size();

				if (num_better_than >= target_better_than) {
					is_success = true;
				}

				if (wrapper->solution->train_new_last_scores.size() >= NUM_LAST_TRACK) {
					wrapper->solution->train_new_last_scores.pop_front();
				}
				wrapper->solution->train_new_last_scores.push_back(global_improvement);
			} else {
				wrapper->solution->train_new_last_scores.push_back(global_improvement);
			}

			is_success = true;
		}

		if (is_success) {
			this->start_state_history.clear();
			this->start_target_val_history.clear();
			this->end_state_history.clear();
			this->end_target_val_history.clear();

			this->curr_ramp = 0;
			this->measure_status = MEASURE_STATUS_N_A;

			this->total_count = 0;
			this->existing_sum_scores = 0.0;
			this->existing_count = 0;
			this->new_sum_scores = 0.0;
			this->new_count = 0;

			this->state = EXPERIMENT_STATE_RAMP;
			this->state_iter = 0;
		} else {
			switch (this->node_context->type) {
			case NODE_TYPE_START:
				{
					StartNode* start_node = (StartNode*)this->node_context;
					start_node->experiment = NULL;
				}
				break;
			case NODE_TYPE_ACTION:
				{
					ActionNode* action_node = (ActionNode*)this->node_context;
					action_node->experiment = NULL;
				}
				break;
			case NODE_TYPE_BRANCH:
				{
					BranchNode* branch_node = (BranchNode*)this->node_context;
					if (this->is_branch) {
						branch_node->branch_experiment = NULL;
					} else {
						branch_node->original_experiment = NULL;
					}
				}
				break;
			}
			delete this;
		}
	}
}
