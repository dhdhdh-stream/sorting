#include "solution.h"

#include <iostream>

#include "action_node.h"
#include "outer_experiment.h"
#include "scope.h"
#include "state.h"

using namespace std;

Solution::Solution() {
	this->outer_experiment = new OuterExperiment();
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

	delete this->outer_experiment;
}

void Solution::init() {
	this->id = (unsigned)time(NULL);

	this->state_counter = 0;

	this->scope_counter = 0;

	Scope* starting_scope = new Scope();
	starting_scope->id = this->scope_counter;
	this->scope_counter++;
	this->scopes[starting_scope->id] = starting_scope;
	
	starting_scope->num_input_states = 0;
	starting_scope->num_local_states = 0;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = starting_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_scope->nodes[0] = starting_noop_node;
	starting_scope->starting_node_id = 0;
	starting_scope->starting_node = starting_noop_node;
	starting_scope->node_counter = 1;

	this->root = starting_scope;

	this->max_depth = 1;
	this->depth_limit = 11;

	this->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + "/solution.txt");

	string id_line;
	getline(input_file, id_line);
	this->id = stoi(id_line);

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
								 state_id,
								 path,
								 name);

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

		scope->id = scope_id;

		this->scopes[scope_id] = scope;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		ifstream scope_save_file;
		scope_save_file.open(path + "saves/" + name + "/scope_" + to_string(it->first) + ".txt");
		it->second->load(scope_save_file);
		scope_save_file.close();
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link();
	}

	string root_id_line;
	getline(input_file, root_id_line);
	this->root = this->scopes[stoi(root_id_line)];

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}

	input_file.close();

	this->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
}

void Solution::clear_verify() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->clear_verify();
	}

	this->verify_key = NULL;
	this->verify_problems.clear();
}

void Solution::success_reset() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->success_reset();
	}

	delete this->outer_experiment;
	this->outer_experiment = new OuterExperiment();
}

void Solution::fail_reset() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->fail_reset();
	}

	delete this->outer_experiment;
	this->outer_experiment = new OuterExperiment();
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "/solution.txt");

	output_file << this->id << endl;

	output_file << this->state_counter << endl;

	output_file << this->states.size() << endl;
	for (map<int, State*>::iterator it = this->states.begin();
			it != this->states.end(); it++) {
		output_file << it->first << endl;
		it->second->save(output_file,
						 path,
						 name);
	}

	output_file << this->scope_counter << endl;

	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;

		ofstream scope_save_file;
		scope_save_file.open(path + "saves/" + name + "/scope_" + to_string(it->first) + ".txt");
		it->second->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->root->id << endl;

	output_file << this->max_depth << endl;

	output_file.close();
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
		it->second->save_for_display(output_file);
	}
}
