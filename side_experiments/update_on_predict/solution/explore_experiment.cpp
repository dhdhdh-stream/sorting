#include "explore_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "network.h"
#include "noop_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

ExploreExperiment::ExploreExperiment(SolutionWrapper* wrapper,
									 int experiment_index,
									 Scope* scope_context,
									 AbstractNode* node_context,
									 bool is_branch,
									 AbstractNode* exit_next_node) {
	this->wrapper = wrapper;
	this->experiment_index = experiment_index;

	this->scope_context = scope_context;
	this->node_context = node_context;
	this->is_branch = is_branch;
	this->exit_next_node = exit_next_node;

	if (this->scope_context->predict_explore_num_stabilize < NEW_SCOPE_NUM_STABILIZE) {
		this->is_signal = false;
	} else {
		this->is_signal = true;
	}

	this->existing_score_network = NULL;
	this->existing_misguess_network = NULL;
	this->new_score_network = NULL;
	this->new_misguess_network = NULL;

	this->try_iter = 0;

	this->state = EXPLORE_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;
}

ExploreExperiment::~ExploreExperiment() {
	switch (this->node_context->type) {
	case NODE_TYPE_NOOP:
		{
			NoopNode* noop_node = (NoopNode*)this->node_context;
			for (int e_index = 0; e_index < (int)noop_node->experiments.size(); e_index++) {
				if (noop_node->experiments[e_index] == this) {
					noop_node->experiments.erase(noop_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;
			for (int e_index = 0; e_index < (int)action_node->experiments.size(); e_index++) {
				if (action_node->experiments[e_index] == this) {
					action_node->experiments.erase(action_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;
			for (int e_index = 0; e_index < (int)scope_node->experiments.size(); e_index++) {
				if (scope_node->experiments[e_index] == this) {
					scope_node->experiments.erase(scope_node->experiments.begin() + e_index);
					break;
				}
			}
		}
		break;
	case NODE_TYPE_BRANCH:
		{
			BranchNode* branch_node = (BranchNode*)this->node_context;
			if (this->is_branch) {
				for (int e_index = 0; e_index < (int)branch_node->branch_experiments.size(); e_index++) {
					if (branch_node->branch_experiments[e_index] == this) {
						branch_node->branch_experiments.erase(branch_node->branch_experiments.begin() + e_index);
						break;
					}
				}
			} else {
				for (int e_index = 0; e_index < (int)branch_node->original_experiments.size(); e_index++) {
					if (branch_node->original_experiments[e_index] == this) {
						branch_node->original_experiments.erase(branch_node->original_experiments.begin() + e_index);
						break;
					}
				}
			}
		}
		break;
	}

	if (this->existing_score_network != NULL) {
		delete this->existing_score_network;
	}
	if (this->existing_misguess_network != NULL) {
		delete this->existing_misguess_network;
	}

	if (this->new_score_network != NULL) {
		delete this->new_score_network;
	}
	if (this->new_misguess_network != NULL) {
		delete this->new_misguess_network;
	}
}

void ExploreExperiment::backprop(vector<double>& obs,
								 double target_val,
								 double misguess) {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
	case EXPLORE_EXPERIMENT_STATE_MEASURE:
		this->existing_score_network->activate(obs);
		double score_error = target_val - this->existing_score_network->output->acti_vals(0);
		this->existing_score_network->backprop(score_error);

		this->existing_misguess_network->activate(obs);
		double misguess_error = misguess - this->existing_misguess_network->output->acti_vals(0);
		this->existing_misguess_network->backprop(misguess_error);

		break;
	}
}

void ExploreExperiment::update() {
	switch (this->state) {
	case EXPLORE_EXPERIMENT_STATE_EXPLORE:
	case EXPLORE_EXPERIMENT_STATE_TRAIN_NEW:
	case EXPLORE_EXPERIMENT_STATE_MEASURE:
		this->existing_score_network->epoch_iter++;
		if (this->existing_score_network->epoch_iter >= UPDATE_EPOCH_SIZE) {
			this->existing_score_network->update();
			this->existing_misguess_network->update();

			this->existing_score_network->epoch_iter = 0;
		}
		break;
	}
}

ExploreExperimentHistory::ExploreExperimentHistory(ExploreExperiment* experiment) {
	this->experiment = experiment;

	this->has_explore = false;
}

ExploreExperimentState::ExploreExperimentState(ExploreExperiment* experiment) {
	this->experiment = experiment;
}
