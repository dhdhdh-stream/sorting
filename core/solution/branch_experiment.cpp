#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "globals.h"
#include "obs_experiment.h"
#include "scale.h"
#include "scope.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"

using namespace std;

BranchExperiment::BranchExperiment(vector<int> scope_context,
								   vector<int> node_context) {
	this->scope_context = scope_context;
	this->node_context = node_context;

	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	this->average_remaining_experiments_from_start = 0.0;
	this->average_instances_per_run = 1.0;

	this->state = BRANCH_EXPERIMENT_STATE_TRAIN_EXISTING;
	this->state_iter = 0;

	this->existing_average_score = parent_scope->average_score;
	this->existing_average_misguess = parent_scope->average_misguess;

	this->new_scope_id = solution->scope_counter;
	solution->scope_counter++;

	this->best_surprise = 1.0;

	uniform_int_distribution<int> recursion_protection_distribution(0, 4);
	if (recursion_protection_distribution(generator) != 0) {
		this->recursion_protection = true;
	} else {
		this->recursion_protection = false;
	}
	this->need_recursion_protection = false;

	this->new_average_score = parent_scope->average_score;
	this->new_average_misguess = parent_scope->average_misguess;
	this->new_misguess_variance = parent_scope->misguess_variance;

	this->branch_existing_score = 0.0;
	this->existing_branch_count = 0;
	this->non_branch_existing_score = 0.0;

	this->branch_new_score = 0.0;
	this->new_branch_count = 0;
	this->non_branch_new_score = 0.0;

	this->pass_through_score = 0.0;

	this->obs_experiment = NULL;
}

BranchExperiment::~BranchExperiment() {
	for (map<int, Scale*>::iterator it = this->existing_starting_input_state_scales.begin();
			it != this->existing_starting_input_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<int, Scale*>::iterator it = this->existing_starting_local_state_scales.begin();
			it != this->existing_starting_local_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<State*, Scale*>::iterator it = this->existing_starting_score_state_scales.begin();
			it != this->existing_starting_score_state_scales.end(); it++) {
		delete it->second;
	}

	for (int s_index = 0; s_index < (int)this->best_actions.size(); s_index++) {
		if (this->best_actions[s_index] != NULL) {
			delete this->best_actions[s_index];
		}
	}

	for (int s_index = 0; s_index < (int)this->best_sequences.size(); s_index++) {
		if (this->best_sequences[s_index] != NULL) {
			delete this->best_sequences[s_index];
		}
	}

	for (map<int, Scale*>::iterator it = this->new_starting_input_state_scales.begin();
			it != this->new_starting_input_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<int, Scale*>::iterator it = this->new_starting_local_state_scales.begin();
			it != this->new_starting_local_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<State*, Scale*>::iterator it = this->new_starting_score_state_scales.begin();
			it != this->new_starting_score_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<State*, Scale*>::iterator it = this->new_starting_experiment_score_state_scales.begin();
			it != this->new_starting_experiment_score_state_scales.end(); it++) {
		delete it->second;
	}

	for (map<int, Scale*>::iterator it = this->new_ending_input_state_scales.begin();
			it != this->new_ending_input_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<int, Scale*>::iterator it = this->new_ending_local_state_scales.begin();
			it != this->new_ending_local_state_scales.end(); it++) {
		delete it->second;
	}
	for (map<State*, Scale*>::iterator it = this->new_ending_score_state_scales.begin();
			it != this->new_ending_score_state_scales.end(); it++) {
		delete it->second;
	}

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		delete this->new_score_states[s_index];
	}

	for (map<State*, pair<Scale*, double>>::iterator it = this->new_score_state_scales.begin();
			it != this->new_score_state_scales.end(); it++) {
		delete it->second.first;
	}

	if (this->obs_experiment != NULL) {
		delete this->obs_experiment;
	}
}

BranchExperimentHistory::BranchExperimentHistory(BranchExperiment* experiment) {
	this->experiment = experiment;
}

BranchExperimentHistory::~BranchExperimentHistory() {
	for (int s_index = 0; s_index < (int)this->sequence_histories.size(); s_index++) {
		if (this->sequence_histories[s_index] != NULL) {
			delete this->sequence_histories[s_index];
		}
	}
}
