#include "scope_experiment.h"

#include <iostream>
#include <sstream>

#include "constants.h"
#include "experiment.h"
#include "helpers.h"
#include "obs_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "solution_wrapper.h"

using namespace std;

void ScopeExperiment::add(SolutionWrapper* wrapper) {
	stringstream ss;
	ss << "timestamp: " << wrapper->solution->timestamp << "; ";
	ss << "this->node_context->parent->id: " << this->node_context->parent->id << "; ";
	ss << "this->node_context->id: " << this->node_context->id << "; ";

	if (this->exit_next_node == NULL) {
		ss << "this->exit_next_node->id: " << -1 << "; ";
	} else {
		ss << "this->exit_next_node->id: " << this->exit_next_node->id << "; ";
	}

	ss << "this->local_improvement: " << this->local_improvement << "; ";
	ss << "this->global_improvement: " << this->global_improvement << "; ";

	double score_average = this->total_sum_scores / this->total_count;
	ss << "score_average: " << score_average << "; ";
	cout << "score_average: " << score_average << endl;
	wrapper->solution->curr_score = score_average;

	wrapper->solution->improvement_history.push_back(score_average);
	wrapper->solution->change_history.push_back(ss.str());

	cout << ss.str() << endl;

	Scope* scope_context = this->node_context->parent;

	wrapper->solution->scopes.push_back(this->new_scope);
	this->new_scope->is_outer = false;
	this->new_scope->id = (int)wrapper->solution->scopes.size()-1;

	this->new_scope->child_scopes = scope_context->child_scopes;
	recursive_add_child(scope_context,
						wrapper,
						this->new_scope);

	ScopeNode* new_scope_node = new ScopeNode();
	new_scope_node->parent = scope_context;
	new_scope_node->id = scope_context->node_counter;
	scope_context->node_counter++;
	scope_context->nodes[new_scope_node->id] = new_scope_node;

	new_scope_node->scope = this->new_scope;

	this->new_scope = NULL;

	ObsNode* new_ending_node = NULL;

	int exit_node_id;
	AbstractNode* exit_node;
	if (this->exit_next_node == NULL) {
		new_ending_node = new ObsNode();
		new_ending_node->parent = scope_context;
		new_ending_node->id = scope_context->node_counter;
		scope_context->node_counter++;

		for (map<int, AbstractNode*>::iterator it = scope_context->nodes.begin();
				it != scope_context->nodes.end(); it++) {
			if (it->second->type == NODE_TYPE_OBS) {
				ObsNode* obs_node = (ObsNode*)it->second;
				if (obs_node->next_node == NULL) {
					obs_node->next_node_id = new_ending_node->id;
					obs_node->next_node = new_ending_node;

					new_ending_node->ancestor_ids.push_back(obs_node->id);

					break;
				}
			}
		}

		scope_context->nodes[new_ending_node->id] = new_ending_node;

		new_ending_node->next_node_id = -1;
		new_ending_node->next_node = NULL;

		exit_node_id = new_ending_node->id;
		exit_node = new_ending_node;
	} else {
		exit_node_id = this->exit_next_node->id;
		exit_node = this->exit_next_node;
	}

	if (this->node_context->next_node != NULL) {
		for (int a_index = 0; a_index < (int)this->node_context->next_node->ancestor_ids.size(); a_index++) {
			if (this->node_context->next_node->ancestor_ids[a_index] == this->node_context->id) {
				this->node_context->next_node->ancestor_ids.erase(
					this->node_context->next_node->ancestor_ids.begin() + a_index);
				break;
			}
		}
	}

	this->node_context->next_node_id = new_scope_node->id;
	this->node_context->next_node = new_scope_node;

	new_scope_node->ancestor_ids.push_back(this->node_context->id);

	new_scope_node->next_node_id = exit_node_id;
	new_scope_node->next_node = exit_node;

	exit_node->ancestor_ids.push_back(new_scope_node->id);
}
