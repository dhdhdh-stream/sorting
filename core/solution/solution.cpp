#include "solution.h"

#include <iostream>
#include <random>

#include "globals.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	delete this->root;

	for (int s_index = 0; s_index < (int)this->scope_dictionary.size(); s_index++) {
		delete this->scope_dictionary[s_index];
	}
}

void Solution::init() {
	this->id_counter = 0;

	this->root = new Scope();
	int starting_num_inputs = 0;
	int starting_num_outputs = 0;
	int starting_sequence_length = 1;
	vector<bool> starting_is_inner_scope{false};
	vector<Scope*> starting_scopes{NULL};
	vector<Action> starting_actions{Action(0.0, ACTION_START)};
	vector<vector<FoldNetwork*>> starting_inner_input_networks{vector<FoldNetwork*>()};
	vector<vector<int>> starting_inner_input_sizes{vector<int>()};
	vector<Network*> starting_scope_scale_mod{NULL};
	vector<int> starting_step_types{STEP_TYPE_STEP};
	vector<Branch*> starting_branches{NULL};
	vector<Fold*> starting_folds{NULL};
	FoldNetwork* starting_score_network = new FoldNetwork(
		1,
		0,
		vector<int>{1},
		20);
	vector<FoldNetwork*> starting_score_networks{starting_score_network};
	vector<double> starting_average_inner_scope_impacts{0.0};
	vector<double> starting_average_local_impacts{0.0};
	vector<double> starting_average_inner_branch_impacts{0.0};
	vector<bool> starting_active_compress{false};
	vector<int> starting_compress_new_sizes{0};
	vector<FoldNetwork*> starting_compress_networks{NULL};
	vector<int> starting_compress_original_sizes{1};
	bool starting_full_last = true;
	this->root->initialize(starting_num_inputs,
						   starting_num_outputs,
						   starting_sequence_length,
						   starting_is_inner_scope,
						   starting_scopes,
						   starting_actions,
						   starting_inner_input_networks,
						   starting_inner_input_sizes,
						   starting_scope_scale_mod,
						   starting_step_types,
						   starting_branches,
						   starting_folds,
						   starting_score_networks,
						   starting_average_inner_scope_impacts,
						   starting_average_local_impacts,
						   starting_average_inner_branch_impacts,
						   0.0,
						   0.0,
						   0.0,
						   0.0,
						   starting_active_compress,
						   starting_compress_new_sizes,
						   starting_compress_networks,
						   starting_compress_original_sizes,
						   starting_full_last);
	this->root->id = -1;

	this->new_sequence_index = 0;

	this->max_depth = 1;
	this->depth_limit = 11;
}

void Solution::load(ifstream& input_file) {
	string id_counter_line;
	getline(input_file, id_counter_line);
	this->id_counter = stoi(id_counter_line);

	string action_dictionary_size_line;
	getline(input_file, action_dictionary_size_line);
	int action_dictionary_size = stoi(action_dictionary_size_line);
	for (int a_index = 0; a_index < action_dictionary_size; a_index++) {
		this->action_dictionary.push_back(Action(input_file));
	}

	for (int a_index = 0; a_index < action_dictionary_size; a_index++) {
		string action_last_success_line;
		getline(input_file, action_last_success_line);
		this->action_last_success.push_back(stoi(action_last_success_line));
	}

	string scope_dictionary_size_line;
	getline(input_file, scope_dictionary_size_line);
	int scope_dictionary_size = stoi(scope_dictionary_size_line);
	for (int s_index = 0; s_index < scope_dictionary_size; s_index++) {
		this->scope_dictionary.push_back(new Scope());
	}
	for (int s_index = 0; s_index < scope_dictionary_size; s_index++) {
		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scope_dictionary[s_index]->load(scope_save_file);
		scope_save_file.close();
	}

	for (int s_index = 0; s_index < scope_dictionary_size; s_index++) {
		string scope_last_success_line;
		getline(input_file, scope_last_success_line);
		this->scope_last_success.push_back(stoi(scope_last_success_line));
	}

	this->root = new Scope();
	ifstream scope_save_file;
	scope_save_file.open("saves/scope_root.txt");
	this->root->load(scope_save_file);
	scope_save_file.close();

	string new_sequence_index_line;
	getline(input_file, new_sequence_index_line);
	this->new_sequence_index = stoi(new_sequence_index_line);

	string max_depth_line;
	getline(input_file, max_depth_line);
	this->max_depth = stoi(max_depth_line);

	if (this->max_depth < 50) {
		this->depth_limit = this->max_depth + 10;
	} else {
		this->depth_limit = (int)(1.2*(double)this->max_depth);
	}
}

void Solution::new_sequence(int& sequence_length,
							vector<int>& new_sequence_types,
							vector<int>& existing_scope_ids,
							vector<int>& existing_action_ids,
							vector<Action>& new_actions,
							bool can_be_empty) {
	// Note: don't refold, and instead try longer sequences to try to unwind to find deeper connections
	geometric_distribution<int> geo_dist(0.2);
	if (can_be_empty) {
		sequence_length = geo_dist(generator);
	} else {
		sequence_length = 1 + geo_dist(generator);
	}
	for (int s_index = 0; s_index < sequence_length; s_index++) {
		if (this->scope_dictionary.size() > 0 && rand()%2 == 0) {
			new_sequence_types.push_back(NEW_SEQUENCE_TYPE_EXISTING_SCOPE);
			existing_scope_ids.push_back(rand()%(int)this->scope_dictionary.size());

			existing_action_ids.push_back(-1);
			new_actions.push_back(Action());
		} else {
			if (this->action_dictionary.size() > 0 && rand()%2 == 0) {
				new_sequence_types.push_back(NEW_SEQUENCE_TYPE_EXISTING_ACTION);
				existing_action_ids.push_back(rand()%(int)this->action_dictionary.size());

				existing_scope_ids.push_back(-1);
				new_actions.push_back(Action());
			} else {
				new_sequence_types.push_back(NEW_SEQUENCE_TYPE_NEW_ACTION);

				normal_distribution<double> norm_dist(0.0, 1.0);
				double write = norm_dist(generator);

				int move = rand()%3;

				new_actions.push_back(Action(write, move));

				existing_scope_ids.push_back(-1);
				existing_action_ids.push_back(-1);
			}
		}
	}
}

void Solution::new_sequence_success(int sequence_length,
									vector<int>& new_sequence_types,
									vector<int>& existing_scope_ids,
									vector<int>& existing_action_ids,
									vector<Action>& new_actions) {
	for (int s_index = 0; s_index < sequence_length; s_index++) {
		if (new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_SCOPE) {
			this->scope_last_success[existing_scope_ids[s_index]] = this->new_sequence_index;
		} else if (new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_EXISTING_ACTION) {
			this->action_last_success[existing_action_ids[s_index]] = this->new_sequence_index;
		} else {
			// new_sequence_types[s_index] == NEW_SEQUENCE_TYPE_NEW_ACTION
			this->action_dictionary.push_back(new_actions[s_index]);
			this->action_last_success.push_back(this->new_sequence_index);
		}
	}
}

void Solution::new_sequence_iter() {
	int a_index = 0;
	while (a_index < (int)this->action_dictionary.size()) {
		if (this->new_sequence_index - this->action_last_success[a_index] > 1000000) {
			this->action_dictionary.erase(this->action_dictionary.begin()+a_index);
			this->action_last_success.erase(this->action_last_success.begin()+a_index);
		} else {
			a_index++;
		}
	}

	this->new_sequence_index++;
}

void Solution::save(ofstream& output_file) {
	output_file << this->id_counter << endl;

	output_file << this->action_dictionary.size() << endl;
	for (int a_index = 0; a_index < (int)this->action_dictionary.size(); a_index++) {
		this->action_dictionary[a_index].save(output_file);
	}

	for (int a_index = 0; a_index < (int)this->action_dictionary.size(); a_index++) {
		output_file << this->action_last_success[a_index] << endl;
	}

	output_file << this->scope_dictionary.size() << endl;
	for (int s_index = 0; s_index < (int)this->scope_dictionary.size(); s_index++) {
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scope_dictionary[s_index]->save(scope_save_file);
		scope_save_file.close();
	}

	for (int s_index = 0; s_index < (int)this->scope_dictionary.size(); s_index++) {
		output_file << this->scope_last_success[s_index] << endl;
	}

	ofstream scope_save_file;
	scope_save_file.open("saves/scope_root.txt");
	this->root->save(scope_save_file);
	scope_save_file.close();

	output_file << this->new_sequence_index << endl;

	output_file << this->max_depth << endl;
}
