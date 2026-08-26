#include "predict_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution_helpers.h"

using namespace std;

void PredictExperiment::train_new_helper(SolutionWrapper* wrapper,
										 bool& is_add) {
	vector<double> new_target_val_histories;
	for (int h_index = 0; h_index < (int)this->existing_state_histories.size(); h_index++) {
		double sum_vals = 0.0;
		for (int r_index = 0; r_index < RUNS_PER_PREDICT; r_index++) {
			Eigen::VectorXf state = this->existing_state_histories[h_index];
			AbstractNode* node_context = this->exit_next_node;

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					ActionNode* generic_action_node = this->scope_context->generic_action_nodes[this->best_indexes[s_index]];
					generic_action_node->predict_step(state,
													  node_context);
				} else {
					ScopeNode* generic_scope_node = this->scope_context->generic_scope_nodes[this->best_indexes[s_index]];
					generic_scope_node->predict_step(state,
													 node_context);
				}
			}

			sum_vals += predict_helper(node_context,
									   state,
									   this->scope_context);
		}

		new_target_val_histories.push_back(sum_vals / RUNS_PER_PREDICT);
	}

	this->new_network = new ScoreNetwork(this->scope_context->num_states);

	uniform_int_distribution<int> new_train_distribution(0, this->existing_state_histories.size()-1);
	for (int iter_index = 0; iter_index < TRAIN_ITERS; iter_index++) {
		int rand_index = new_train_distribution(generator);

		this->new_network->activate(this->existing_state_histories[rand_index]);

		this->new_network->init_backprop(new_target_val_histories[rand_index]);

		if ((iter_index+1)%INIT_EPOCH_SIZE == 0) {
			this->new_network->init_update();
		}
	}
	for (int s_index = 0; s_index < (int)this->new_network->state_input->errors.size(); s_index++) {
		this->new_network->state_input->errors(s_index) = 0.0;
	}

	double existing_sum_vals = 0.0;
	double new_sum_vals = 0.0;
	int count = 0;
	for (int h_index = 0; h_index < (int)this->existing_state_histories.size(); h_index++) {
		this->existing_network->activate(this->existing_state_histories[h_index]);
		double existing_predicted = this->existing_network->output->acti_vals[0];
		this->new_network->activate(this->existing_state_histories[h_index]);
		double new_predicted = this->new_network->output->acti_vals[0];

		// temp
		if (h_index < 10) {
			cout << h_index << endl;
			cout << "existing_predicted: " << existing_predicted << endl;
			cout << "new_predicted: " << new_predicted << endl;
			cout << "this->existing_target_val_histories[h_index]: " << this->existing_target_val_histories[h_index] << endl;
			cout << "new_target_val_histories[h_index]: " << new_target_val_histories[h_index] << endl;
		}

		if (new_predicted >= existing_predicted) {
			existing_sum_vals += this->existing_target_val_histories[h_index];
			new_sum_vals += new_target_val_histories[h_index];
			count++;
		}
	}
	double existing_average = existing_sum_vals / (double)count;
	double new_average = new_sum_vals / (double)count;
	double average_ratio = (double)count / (double)this->existing_state_histories.size();
	double local_improvement = (new_average - existing_average) * average_ratio;

	double average_instances_per_run;
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			average_instances_per_run = noop_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			average_instances_per_run = action_node->average_instances_per_run;
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			average_instances_per_run = scope_node->average_instances_per_run;
		}
		break;
	default:
	// case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				average_instances_per_run = branch_node->branch_average_instances_per_run;
			} else {
				average_instances_per_run = branch_node->original_average_instances_per_run;
			}
		}
		break;
	}
	double global_improvement = average_instances_per_run * local_improvement;

	// temp
	cout << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			cout << " " << this->best_indexes[s_index];
		} else {
			cout << " E" << this->scope_context->child_scopes[this->best_indexes[s_index]]->id;
		}
	}
	cout << endl;
	cout << "count: " << count << endl;
	cout << "local_improvement: " << local_improvement << endl;
	cout << "global_improvement: " << global_improvement << endl;

	if (local_improvement > 0.0) {
		bool is_success = false;
		if (this->scope_context->predict_last_scores.size() >= MIN_NUM_LAST_TRACK) {
			int num_better_than = 0;
			for (list<double>::iterator it = this->scope_context->predict_last_scores.begin();
					it != this->scope_context->predict_last_scores.end(); it++) {
				if (global_improvement >= *it) {
					num_better_than++;
				}
			}

			double target_better_than = LAST_BETTER_THAN_RATIO * (double)this->scope_context->predict_last_scores.size();

			if (num_better_than >= target_better_than) {
				is_success = true;
			}

			if (this->scope_context->predict_last_scores.size() >= NUM_LAST_TRACK) {
				this->scope_context->predict_last_scores.pop_front();
			}
			this->scope_context->predict_last_scores.push_back(global_improvement);
		} else {
			this->scope_context->predict_last_scores.push_back(global_improvement);
		}

		#if defined(MDEBUG) && MDEBUG
		if (is_success || rand()%3 != 0) {
		#else
		if (is_success) {
		#endif /* MDEBUG */
			is_add = true;

			add(wrapper);
		} else {
			delete this;
		}
	} else {
		delete this;
	}
}
