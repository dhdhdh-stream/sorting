#include "solution_wrapper.h"

#include <iostream>

#include "action_node.h"
#include "constants.h"
#include "globals.h"
#include "helpers.h"
#include "problem.h"
#include "obs_node.h"
#include "scope.h"
#include "signal.h"
#include "solution.h"
#include "start_node.h"

using namespace std;

const int INIT_NUM_ACTIONS = 10;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->iter = 0;

	this->solution = new Solution();
	{
		this->solution->timestamp = 0;
		this->solution->curr_score = 0.0;

		/**
		 * - even though scopes[0] will not be reused, still good to start with:
		 *   - if artificially add empty scopes, may be difficult to extend from
		 *     - and will then junk up explore
		 *   - new scopes will be created from the reusable portions anyways
		 */

		Scope* new_scope = new Scope();
		new_scope->id = this->solution->scopes.size();
		this->solution->scopes.push_back(new_scope);

		new_scope->node_counter = 0;

		StartNode* start_node = new StartNode();
		start_node->parent = new_scope;
		start_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[start_node->id] = start_node;

		vector<ActionNode*> action_nodes;
		uniform_int_distribution<int> action_distribution(0, problem_type->num_possible_actions()-1);
		for (int n_index = 0; n_index < INIT_NUM_ACTIONS; n_index++) {
			ActionNode* action_node = new ActionNode();
			action_node->parent = new_scope;
			action_node->id = new_scope->node_counter;
			new_scope->node_counter++;
			new_scope->nodes[action_node->id] = action_node;

			action_node->action = action_distribution(generator);

			action_nodes.push_back(action_node);
		}

		ObsNode* end_node = new ObsNode();
		end_node->parent = new_scope;
		end_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[end_node->id] = end_node;

		start_node->next_node_id = action_nodes[0]->id;
		start_node->next_node = action_nodes[0];

		action_nodes[0]->ancestor_ids.push_back(start_node->id);

		for (int a_index = 1; a_index < (int)action_nodes.size(); a_index++) {
			action_nodes[a_index-1]->next_node_id = action_nodes[a_index]->id;
			action_nodes[a_index-1]->next_node = action_nodes[a_index];

			action_nodes[a_index]->ancestor_ids.push_back(action_nodes[a_index-1]->id);
		}

		action_nodes.back()->next_node_id = end_node->id;
		action_nodes.back()->next_node = end_node;

		end_node->ancestor_ids.push_back(action_nodes.back()->id);

		end_node->next_node_id = -1;
		end_node->next_node = NULL;

		clean_scope(new_scope,
					this);

		this->solution->state = SOLUTION_STATE_NON_OUTER;
	}

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	create_experiments(this);
}

SolutionWrapper::SolutionWrapper(std::string path,
								 std::string name) {
	this->iter = 0;

	ifstream input_file;
	input_file.open(path + name);

	this->solution = new Solution();
	this->solution->load(input_file);

	input_file.close();

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	create_experiments(this);
}

SolutionWrapper::~SolutionWrapper() {
	delete this->solution;
}

bool SolutionWrapper::is_done() {
	return this->solution->timestamp == -1;
}

void SolutionWrapper::clean_scopes() {
	this->solution->clean_scopes();
}

void SolutionWrapper::combine(string other_path,
							  string other_name) {
	ifstream input_file;
	input_file.open(other_path + other_name);

	Solution* other = new Solution();
	other->load(input_file);

	input_file.close();

	this->solution->outer_root_scope_ids.push_back(this->solution->outer_scopes.size());

	for (int o_index = 0; o_index < (int)other->scopes.size(); o_index++) {
		this->solution->outer_scopes.push_back(other->scopes[o_index]);

		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			this->solution->scopes[s_index]->child_scopes.push_back(other->scopes[o_index]);
		}
	}

	other->scopes.clear();

	delete other;

	for (int scope_index = 0; scope_index < (int)this->solution->outer_scopes.size(); scope_index++) {
		this->solution->outer_scopes[scope_index]->is_outer = true;
		this->solution->outer_scopes[scope_index]->id = scope_index;
	}

	this->solution->last_scores.clear();

	this->solution->state = SOLUTION_STATE_OUTER;
	this->solution->timestamp = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void SolutionWrapper::save_for_display(string path,
									   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
