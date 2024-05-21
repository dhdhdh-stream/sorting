#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "eval.h"
#include "globals.h"
#include "info_scope.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->timestamp_score = original->timestamp_score;

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

	this->curr_average_score = original->curr_average_score;

	this->max_num_actions = original->max_num_actions;
	this->num_actions_limit = original->num_actions_limit;

	uniform_int_distribution<int> explore_id_distribution(0, (int)this->scopes.size()-1);
	this->explore_id = explore_id_distribution(generator);
	// if (this->scopes[this->explore_id]->eval->score_input_node_contexts.size() == 0) {
	// 	this->explore_type = EXPLORE_TYPE_EVAL;
	// } else {
	// 	uniform_int_distribution<int> explore_type_distribution(0, 1);
	// 	this->explore_type = explore_type_distribution(generator);
	// }
	// temp
	if (original->explore_type == EXPLORE_TYPE_EVAL) {
		this->explore_type = EXPLORE_TYPE_SCORE;
	} else {
		this->explore_type = EXPLORE_TYPE_EVAL;
	}
	this->explore_scope_max_num_actions = 1;
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
	this->timestamp_score = -1.0;

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

	new_scope->eval = new Eval(new_scope);
	new_scope->eval->init();

	this->curr_average_score = 1.0;

	this->max_num_actions = 1;
	this->num_actions_limit = 40;

	this->explore_id = 0;
	this->explore_type = EXPLORE_TYPE_EVAL;
	this->explore_average_instances_per_run = 1.0;
	this->explore_scope_max_num_actions = 1;
	this->explore_scope_local_average_num_actions = 1.0;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string timestamp_score_line;
	getline(input_file, timestamp_score_line);
	this->timestamp_score = stod(timestamp_score_line);

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

	string curr_average_score_line;
	getline(input_file, curr_average_score_line);
	this->curr_average_score = stod(curr_average_score_line);

	string max_num_actions_line;
	getline(input_file, max_num_actions_line);
	this->max_num_actions = stoi(max_num_actions_line);

	this->num_actions_limit = 20*this->max_num_actions + 20;

	string explore_id_line;
	getline(input_file, explore_id_line);
	this->explore_id = stoi(explore_id_line);

	string explore_type_line;
	getline(input_file, explore_type_line);
	this->explore_type = stoi(explore_type_line);

	string explore_average_instances_per_run_line;
	getline(input_file, explore_average_instances_per_run_line);
	this->explore_average_instances_per_run = stod(explore_average_instances_per_run_line);

	string explore_scope_max_num_actions_line;
	getline(input_file, explore_scope_max_num_actions_line);
	this->explore_scope_max_num_actions = stoi(explore_scope_max_num_actions_line);

	string explore_scope_local_average_num_actions_line;
	getline(input_file, explore_scope_local_average_num_actions_line);
	this->explore_scope_local_average_num_actions = stod(explore_scope_local_average_num_actions_line);

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

	this->verify_key = NULL;
	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->timestamp_score << endl;

	output_file << this->scopes.size() << endl;
	output_file << this->info_scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}
	for (int i_index = 0; i_index < (int)this->info_scopes.size(); i_index++) {
		this->info_scopes[i_index]->save(output_file);
	}

	output_file << this->curr_average_score << endl;

	output_file << this->max_num_actions << endl;

	output_file << this->explore_id << endl;
	output_file << this->explore_type << endl;
	output_file << this->explore_average_instances_per_run << endl;
	output_file << this->explore_scope_max_num_actions << endl;
	output_file << this->explore_scope_local_average_num_actions << endl;

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
		output_file << this->info_scopes[i_index]->state << endl;
		if (this->info_scopes[i_index]->state == INFO_SCOPE_STATE_NA) {
			this->info_scopes[i_index]->subscope->save_for_display(output_file);
		}
	}
}
