#include "solution.h"

#include "action_node.h"
#include "state_network.h"

using namespace std;

Solution::Solution() {
	this->average_score = 0.0;

	// starting action ACTION_START
	ActionNode* starting_node = new ActionNode(vector<bool>(),
											   vector<int>(),
											   vector<StateNetwork*>(),
											   new StateNetwork(1,
																0,
																0,
																0,
																0,
																20));
	this->scopes.push_back(new Scope(0,
									 0,
									 false,
									 NULL,
									 NULL,
									 vector<AbstractNode*>{starting_node}));
	this->scopes[0]->id = 0;

	this->max_depth = 1;
	this->depth_limit = 11;
}

Solution::Solution(ifstream& input_file) {
	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes.push_back(new Scope(scope_save_file));
		scope_save_file.close();
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
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->average_score << endl;

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes[s_index]->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->max_depth << endl;
}
