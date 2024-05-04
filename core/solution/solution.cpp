#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "eval.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int TRAVERSE_ITERS = 5;
const int GENERALIZE_ITERS = 5;
#else
const int TRAVERSE_ITERS = 20;
const int GENERALIZE_ITERS = 10;
#endif /* MDEBUG */

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_average_score = original->curr_average_score;
	this->average_num_actions = original->average_num_actions;

	this->state = original->state;
	this->state_iter = original->state_iter;

	this->num_actions_until_experiment = original->num_actions_until_experiment;
	this->num_actions_until_random = original->num_actions_until_random;

	this->current = new Scope();
	this->current->id = -1;
	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}
	for (int i_index = 0; i_index < (int)original->info_scopes.size(); i_index++) {
		InfoScope* scope = new InfoScope();
		scope->id = i_index;
		this->info_scopes.push_back(scope);
	}

	this->current->copy_from(original->current,
							 this);
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->copy_from(original->info_scopes[i_index],
											  this);
	}

	this->current->link(this);
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->link(this);
	}

	this->eval = new Eval(original->eval);

	this->max_depth = original->max_depth;
	this->depth_limit = original->depth_limit;

	this->max_num_actions = original->max_num_actions;
	this->num_actions_limit = original->num_actions_limit;
}

Solution::~Solution() {
	delete this->current;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		delete this->info_scopes[i_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_average_score = -1.0;
	this->average_num_actions = 0.0;

	this->state = SOLUTION_STATE_TRAVERSE;
	#if defined(MDEBUG) && MDEBUG
	this->state_iter = 0;
	#else
	/**
	 * - for initial, start at 10
	 */
	this->state_iter = 10;
	#endif /* MDEBUG */

	this->num_actions_until_experiment = -1;
	this->num_actions_until_random = -1;

	this->current = new Scope();
	this->current->id = -1;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = this->current;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	this->current->nodes[0] = starting_noop_node;
	this->current->node_counter = 1;

	this->eval = new Eval();
	this->eval->init();

	this->max_depth = 1;
	this->depth_limit = 11;

	this->max_num_actions = 1;
	this->num_actions_limit = 40;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_average_score_line;
	getline(input_file, curr_average_score_line);
	this->curr_average_score = stod(curr_average_score_line);

	string average_num_actions_line;
	getline(input_file, average_num_actions_line);
	this->average_num_actions = stod(average_num_actions_line);

	string state_line;
	getline(input_file, state_line);
	this->state = stoi(state_line);

	string state_iter_line;
	getline(input_file, state_iter_line);
	this->state_iter = stoi(state_iter_line);

	if (this->state == SOLUTION_STATE_TRAVERSE) {
		this->num_actions_until_experiment = -1;
	} else if (this->state == SOLUTION_STATE_GENERALIZE) {
		uniform_int_distribution<int> next_distribution(0, (int)(2.0 * this->average_num_actions));
		this->num_actions_until_experiment = 1 + next_distribution(generator);
	}
	this->num_actions_until_random = -1;

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	string num_info_scopes_line;
	getline(input_file, num_info_scopes_line);
	int num_info_scopes = stoi(num_info_scopes_line);

	this->current = new Scope();
	this->current->id = -1;
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}
	for (int i_index = 0; i_index < num_info_scopes; i_index++) {
		InfoScope* scope = new InfoScope();
		scope->id = i_index;
		this->info_scopes.push_back(scope);
	}

	this->current->load(input_file);
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->load(input_file);
	}

	this->current->link(this);
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->link(this);
	}

	this->eval = new Eval();
	this->eval->load(input_file);

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}

	string max_num_actions_line;
	getline(input_file, max_num_actions_line);
	this->max_num_actions = stoi(max_num_actions_line);

	this->num_actions_limit = 20*this->max_num_actions + 20;

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_key = NULL;
	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::increment() {
	this->state_iter++;
	if (this->state == SOLUTION_STATE_TRAVERSE
			&& this->state_iter >= TRAVERSE_ITERS) {
		Scope* new_scope = new Scope();
		new_scope->id = this->scopes.size();
		this->scopes.push_back(new_scope);

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = new_scope;
		starting_noop_node->id = 0;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		new_scope->nodes[0] = starting_noop_node;
		new_scope->node_counter = 1;

		this->state = SOLUTION_STATE_GENERALIZE;
		this->state_iter = 0;

		uniform_int_distribution<int> next_distribution(0, (int)(2.0 * this->average_num_actions));
		this->num_actions_until_experiment = 1 + next_distribution(generator);
		this->num_actions_until_random = -1;
	} else if (this->state == SOLUTION_STATE_GENERALIZE
			&& this->state_iter >= GENERALIZE_ITERS) {
		delete this->current;
		this->current = new Scope();
		this->current->id = -1;

		ActionNode* starting_noop_node = new ActionNode();
		starting_noop_node->parent = this->current;
		starting_noop_node->id = 0;
		starting_noop_node->action = Action(ACTION_NOOP);
		starting_noop_node->next_node_id = -1;
		starting_noop_node->next_node = NULL;
		this->current->nodes[0] = starting_noop_node;
		this->current->node_counter = 1;

		this->state = SOLUTION_STATE_TRAVERSE;
		this->state_iter = 0;

		this->num_actions_until_experiment = -1;
		this->num_actions_until_random = -1;
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->curr_average_score << endl;
	output_file << this->average_num_actions << endl;

	output_file << this->state << endl;
	output_file << this->state_iter << endl;

	output_file << this->scopes.size() << endl;
	output_file << this->info_scopes.size() << endl;

	this->current->save(output_file);
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->save(output_file);
	}

	this->eval->save(output_file);

	output_file << this->max_depth << endl;

	output_file << this->max_num_actions << endl;

	output_file.close();

	string oldname = path + "saves/" + name + "_temp.txt";
	string newname = path + "saves/" + name + ".txt";
	rename(oldname.c_str(), newname.c_str());
}

// TODO: info_scopes
void Solution::save_for_display(ofstream& output_file) {
	this->current->save_for_display(output_file);

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
