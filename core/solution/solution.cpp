#include "solution.h"

#include "action_node.h"
#include "scope.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = (unsigned)time(NULL);

	Scope* starting_scope = new Scope();
	starting_scope->id = this->scopes.size();
	this->scopes.push_back(starting_scope);

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

	this->curr_num_improvements = 0;
	this->previous_generation_index = -1;

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

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file);
	}
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link();
	}

	string curr_num_improvements_line;
	getline(input_file, curr_num_improvements_line);
	this->curr_num_improvements = stoi(curr_num_improvements_line);

	string previous_generation_index_line;
	getline(input_file, previous_generation_index_line);
	this->previous_generation_index = stoi(previous_generation_index_line);

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
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_key = NULL;
	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::reset() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->reset();
	}
}

void Solution::increment() {
	this->curr_num_improvements++;
	if (this->curr_num_improvements >= IMPROVEMENTS_PER_RUN) {
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
		new_scope->default_starting_node_id = 0;
		new_scope->default_starting_node = starting_noop_node;
		new_scope->node_counter = 1;
		
		this->curr_num_improvements = 0;

		if ((int)this->scopes.size() > this->previous_generation_index+1 + GENERATION_SIZE) {
			this->previous_generation_index += GENERATION_SIZE;
		}
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->curr_num_improvements << endl;
	output_file << this->previous_generation_index << endl;

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
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
