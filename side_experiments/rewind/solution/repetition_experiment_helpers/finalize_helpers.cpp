#include "repetition_experiment.h"

#include <iostream>
#include <sstream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_helpers.h"
#include "solution_wrapper.h"

using namespace std;

void RepetitionExperiment::clean() {
	this->node_context->experiment = NULL;
}

void RepetitionExperiment::add(SolutionWrapper* wrapper) {
	stringstream ss;
	ss << "timestamp: " << wrapper->solution->timestamp << "; ";
	ss << "RepetitionExperiment" << "; ";
	ss << "this->node_context->parent->id: " << this->node_context->parent->id << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";

	ss << "this->local_improvement: " << this->local_improvement << "; ";
	ss << "this->global_improvement: " << this->global_improvement << "; ";
	ss << "this->score_standard_deviation: " << this->score_standard_deviation << "; ";
	ss << "this->new_scores.size(): " << this->new_scores.size() << "; ";

	wrapper->solution->improvement_history.push_back(calc_new_score());
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->parent = this->node_context->parent;
	new_scope_node->id = this->node_context->parent->node_counter;
	this->node_context->parent->node_counter++;

	new_scope_node->scope = this->new_scope;

	wrapper->solution->scopes.push_back(this->new_scope);
	this->new_scope->is_outer = false;
	this->new_scope->id = (int)wrapper->solution->scopes.size()-1;

	recursive_add_child(this->scope_context,
						wrapper,
						this->new_scope);

	this->new_scope = NULL;

	switch (this->node_context->type) {
	case NODE_TYPE_ACTION:
		{
			ActionNode* action_node = (ActionNode*)this->node_context;

			for (int a_index = 0; a_index < (int)action_node->next_node->ancestor_ids.size(); a_index++) {
				if (action_node->next_node->ancestor_ids[a_index] == action_node->id) {
					action_node->next_node->ancestor_ids.erase(
						action_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			action_node->next_node_id = new_scope_node->id;
			action_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(action_node->id);
		}
		break;
	case NODE_TYPE_SCOPE:
		{
			ScopeNode* scope_node = (ScopeNode*)this->node_context;

			for (int a_index = 0; a_index < (int)scope_node->next_node->ancestor_ids.size(); a_index++) {
				if (scope_node->next_node->ancestor_ids[a_index] == scope_node->id) {
					scope_node->next_node->ancestor_ids.erase(
						scope_node->next_node->ancestor_ids.begin() + a_index);
					break;
				}
			}

			scope_node->next_node_id = new_scope_node->id;
			scope_node->next_node = new_scope_node;

			new_scope_node->ancestor_ids.push_back(scope_node->id);
		}
		break;
	}

	new_scope_node->next_node_id = this->exit_next_node->id;
	new_scope_node->next_node = this->exit_next_node;

	this->exit_next_node->ancestor_ids.push_back(new_scope_node->id);
}

double RepetitionExperiment::calc_new_score() {
	return this->total_sum_scores / this->total_count;
}
