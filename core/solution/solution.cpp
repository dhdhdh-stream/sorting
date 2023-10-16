#include "solution.h"

#include <iostream>

#include "action_node.h"
#include "scale.h"
#include "scope.h"
#include "state.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

void Solution::init() {
	this->average_score = 0.0;

	this->state_counter = 0;

	Scope* starting_scope = new Scope();
	starting_scope->id = 0;
	starting_scope->num_input_states = 0;
	starting_scope->num_local_states = 0;
	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_scope->nodes.push_back(starting_noop_node);
	starting_scope->average_score = 0.0;
	starting_scope->score_variance = 1.0;
	starting_scope->average_misguess = 0.0;
	starting_scope->misguess_variance = 1.0;
	this->scope_counter = 1;
	this->scopes[0] = starting_scope;

	this->max_depth = 1;
	this->depth_limit = 11;
}

void Solution::load(ifstream& input_file) {
	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string state_counter_line;
	getline(input_file, state_counter_line);
	this->state_counter = stoi(state_counter_line);

	string num_states_line;
	getline(input_file, num_states_line);
	int num_states = stoi(num_states_line);
	for (int s_index = 0; s_index < num_states; s_index++) {
		string state_id_line;
		getline(input_file, state_id_line);
		int state_id = stoi(state_id_line);

		State* state = new State(input_file,
								 state_id);

		this->states[state_id] = state;
	}

	string scope_counter_line;
	getline(input_file, scope_counter_line);
	this->scope_counter = stoi(scope_counter_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		Scope* scope = new Scope();

		this->scopes[scope_id] = scope;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(it->first) + ".txt");
		it->second->load(scope_save_file,
						 it->first);
		scope_save_file.close();
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link_score_state_nodes();
	}

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}
}

Solution::~Solution() {
	for (map<int, State*>::iterator it = this->states.begin();
			it != this->states.end(); it++) {
		delete it->second;
	}

	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		delete it->second;
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->average_score << endl;

	output_file << this->state_counter << endl;

	output_file << this->states.size() << endl;
	for (map<int, State*>::iterator it = this->states.begin();
			it != this->states.end(); it++) {
		output_file << it->first << endl;
		it->second->save(output_file);
	}

	output_file << this->scope_counter << endl;

	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;

		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(it->first) + ".txt");
		it->second->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->max_depth << endl;
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
		it->second->save_for_display(output_file);
	}
}
