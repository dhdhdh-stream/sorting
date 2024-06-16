#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->average_score = original->average_score;
	this->next_possible_new_scope_timestamp = original->next_possible_new_scope_timestamp;

	this->last_updated_scope_id = -1;
	this->last_new_scope_id = -1;

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

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->copy_from(original->info_scopes[i_index],
											  this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->link(this);
	}

	this->max_num_actions = original->max_num_actions;
	this->num_actions_limit = original->num_actions_limit;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		delete this->info_scopes[i_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->average_score = -1.0;
	this->next_possible_new_scope_timestamp = 0;

	this->last_updated_scope_id = 0;
	this->last_new_scope_id = -1;

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

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string next_possible_new_scope_timestamp_line;
	getline(input_file, next_possible_new_scope_timestamp_line);
	this->next_possible_new_scope_timestamp = stoi(next_possible_new_scope_timestamp_line);

	string last_updated_scope_id_line;
	getline(input_file, last_updated_scope_id_line);
	this->last_updated_scope_id = stoi(last_updated_scope_id_line);

	string last_new_scope_id_line;
	getline(input_file, last_new_scope_id_line);
	this->last_new_scope_id = stoi(last_new_scope_id_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	string num_info_scopes_line;
	getline(input_file, num_info_scopes_line);
	int num_info_scopes = stoi(num_info_scopes_line);

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

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->load(input_file);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->link(this);
	}

	string max_num_actions_line;
	getline(input_file, max_num_actions_line);
	this->max_num_actions = stoi(max_num_actions_line);

	#if defined(MDEBUG) && MDEBUG
	this->num_actions_limit = 2*this->max_num_actions + 10;
	#else
	this->num_actions_limit = 10*this->max_num_actions + 10;
	#endif /* MDEBUG */

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->average_score << endl;
	output_file << this->next_possible_new_scope_timestamp << endl;

	output_file << this->last_updated_scope_id << endl;
	output_file << this->last_new_scope_id << endl;

	output_file << this->scopes.size() << endl;
	output_file << this->info_scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->save(output_file);
	}

	output_file << this->max_num_actions << endl;

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

	output_file << this->info_scopes.size() << endl;
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->save_for_display(output_file);
	}
}
