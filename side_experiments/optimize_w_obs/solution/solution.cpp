#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "globals.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double STARTING_TIME_PENALTY = 0.001;

#if defined(MDEBUG) && MDEBUG
const int COMMIT_ITERS = 4;
const int RESET_ITERS = 20;
#else
const int COMMIT_ITERS = 20;
const int RESET_ITERS = 100;
#endif /* MDEBUG */

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_score = original->curr_score;

	this->curr_true_score = original->curr_true_score;
	this->best_true_score = original->best_true_score;
	this->best_true_score_timestamp = original->best_true_score_timestamp;
	this->curr_time_penalty = original->curr_time_penalty;

	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = -1.0;

	this->curr_true_score = -1.0;
	this->best_true_score = -1.0;
	this->best_true_score_timestamp = 0;
	this->curr_time_penalty = STARTING_TIME_PENALTY;

	/**
	 * - even though scopes[0] will not be reused, still good to start with:
	 *   - if artificially add empty scopes, may be difficult to extend from
	 *     - and will then junk up explore
	 *   - new scopes will be created from the reusable portions anyways
	 */

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->node_counter = 0;
	this->scopes.push_back(new_scope);

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_noop_node->average_instances_per_run = 1.0;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	commit(this);
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string curr_true_score_line;
	getline(input_file, curr_true_score_line);
	this->curr_true_score = stod(curr_true_score_line);

	string best_true_score_line;
	getline(input_file, best_true_score_line);
	this->best_true_score = stod(best_true_score_line);

	string best_true_score_timestamp_line;
	getline(input_file, best_true_score_timestamp_line);
	this->best_true_score_timestamp = stoi(best_true_score_timestamp_line);

	string curr_time_penalty_line;
	getline(input_file, curr_time_penalty_line);
	this->curr_time_penalty = stod(curr_time_penalty_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file,
									this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::clean_inputs(int scope_id,
							int node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean_inputs(scope_id,
											node_id);
	}
}

void Solution::check_commit() {
	if (this->timestamp % COMMIT_ITERS) {
		commit(this);
	}
}

void Solution::check_reset() {
	if (this->timestamp % RESET_ITERS == 0) {
		for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
			delete this->scopes[s_index];
		}
		this->scopes.clear();

		this->curr_score = -1.0;

		this->curr_true_score = -1.0;
		this->best_true_score = -1.0;
		this->best_true_score_timestamp = 0;
		this->curr_time_penalty = STARTING_TIME_PENALTY;

		Scope* new_scope = new Scope();
		new_scope->id = this->scopes.size();
		new_scope->node_counter = 0;
		this->scopes.push_back(new_scope);

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = new_scope;
		starting_noop_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		starting_noop_node->average_instances_per_run = 1.0;
		new_scope->nodes[starting_noop_node->id] = starting_noop_node;
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->curr_true_score << endl;
	output_file << this->best_true_score << endl;
	output_file << this->best_true_score_timestamp << endl;
	output_file << this->curr_time_penalty << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file.close();

	string oldname = path + "saves/" + name + "_temp.txt";
	string newname = path + "saves/" + name + ".txt";
	rename(oldname.c_str(), newname.c_str());
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
