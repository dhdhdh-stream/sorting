#include "solution.h"

#include "action_node.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		delete it->second;
	}
}

void Solution::init() {
	this->timestamp = (unsigned)time(NULL);

	this->scope_counter = 0;

	Scope* starting_scope = new Scope();
	starting_scope->id = this->scope_counter;
	this->scope_counter++;
	this->scopes[starting_scope->id] = starting_scope;

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = starting_scope;
	starting_noop_node->id = 0;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_scope->nodes[0] = starting_noop_node;
	starting_scope->default_starting_node_id = 0;
	starting_scope->default_starting_node = starting_noop_node;
	starting_scope->node_counter = 1;

	this->root = starting_scope;
	this->root_num_changes = 0;

	this->throw_counter = 0;

	this->max_depth = 1;
	this->depth_limit = 11;

	this->max_num_actions = 1;
	this->num_actions_limit = 40;

	this->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

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
		it->second->load(input_file);
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->link();
	}

	string root_id_line;
	getline(input_file, root_id_line);
	this->root = this->scopes[stoi(root_id_line)];

	string root_num_changes_line;
	getline(input_file, root_num_changes_line);
	this->root_num_changes = stoi(root_num_changes_line);

	string throw_counter_line;
	getline(input_file, throw_counter_line);
	this->throw_counter = stoi(throw_counter_line);

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

	this->curr_num_datapoints = STARTING_NUM_DATAPOINTS;
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

void Solution::success_reset() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->success_reset();
	}
}

void Solution::fail_reset() {
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->fail_reset();
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;

	output_file << this->scope_counter << endl;

	output_file << this->scopes.size() << endl;
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		output_file << it->first << endl;
	}
	for (map<int, Scope*>::iterator it = this->scopes.begin();
			it != this->scopes.end(); it++) {
		it->second->save(output_file);
	}

	output_file << this->root->id << endl;
	output_file << this->root_num_changes << endl;

	output_file << this->throw_counter << endl;

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
