#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_average_score = original->curr_average_score;

	for (map<int, Scope*>::iterator it = original->scopes.begin();
			it != original->scopes.end(); it++) {
		Scope* scope = new Scope();
		scope->id = it->first;
		this->scopes[it->first] = scope;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->copy_from(original->scopes[it->first],
							  this);
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link(this);
	}

	this->scope_counter = original->scope_counter;

	this->max_depth = original->max_depth;
	this->depth_limit = original->depth_limit;

	this->max_num_actions = original->max_num_actions;
	this->num_actions_limit = original->num_actions_limit;
}

Solution::~Solution() {
	/**
	 * - clear experiments first because of new scope check
	 */
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		for (map<int, AbstractNode*>::iterator node_it = it->second->nodes.begin();
				node_it != it->second->nodes.end(); node_it++) {
			for (int e_index = 0; e_index < (int)node_it->second->experiments.size(); e_index++) {
				delete node_it->second->experiments[e_index];
			}
			node_it->second->experiments.clear();
		}
	}

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		delete it->second;
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_average_score = -1.0;

	Scope* starting_scope = new Scope();
	starting_scope->id = 0;
	this->scopes[0] = starting_scope;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = starting_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_scope->nodes[0] = starting_noop_node;
	starting_scope->node_counter = 1;

	this->scope_counter = 1;

	this->max_depth = 1;
	this->depth_limit = 11;

	// this->max_num_actions = 1;
	// this->num_actions_limit = 40;
	this->max_num_actions = 5;
	this->num_actions_limit = 120;
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

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		string id_line;
		getline(input_file, id_line);
		int scope_id = stoi(id_line);

		Scope* scope = new Scope();
		scope->id = scope_id;
		this->scopes[scope_id] = scope;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->load(input_file,
						 this);
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link(this);
	}

	string scope_counter_line;
	getline(input_file, scope_counter_line);
	this->scope_counter = stoi(scope_counter_line);

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
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->clear_verify();
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
	output_file << this->curr_average_score << endl;

	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->save(output_file);
	}

	output_file << this->scope_counter << endl;

	output_file << this->max_depth << endl;

	output_file << this->max_num_actions << endl;

	output_file.close();

	string oldname = path + "saves/" + name + "_temp.txt";
	string newname = path + "saves/" + name + ".txt";
	rename(oldname.c_str(), newname.c_str());
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
		it->second->save_for_display(output_file);
	}
}
