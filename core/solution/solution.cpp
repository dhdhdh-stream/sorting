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
	vector<Action> starting_actions{Action(ACTION_START)};
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

	this->max_depth = 1;
	this->depth_limit = 11;
}

void Solution::load(ifstream& input_file) {
	string id_counter_line;
	getline(input_file, id_counter_line);
	this->id_counter = stoi(id_counter_line);

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

	this->root = new Scope();
	ifstream scope_save_file;
	scope_save_file.open("saves/scope_root.txt");
	this->root->load(scope_save_file);
	scope_save_file.close();

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
							vector<bool>& is_inner_scope,
							vector<int>& existing_scope_ids,
							vector<Action>& actions,
							bool can_be_empty) {
	// Note: don't refold, and instead try longer sequences to try to unwind to find deeper connections
	geometric_distribution<int> geo_dist(0.2);
	if (can_be_empty) {
		sequence_length = geo_dist(generator);
		if (sequence_length == 0) {
			if (rand()%10 != 0) {
				sequence_length++;
			}
		}
	} else {
		sequence_length = 1 + geo_dist(generator);
	}
	for (int s_index = 0; s_index < sequence_length; s_index++) {
		if (this->scope_dictionary.size() > 0 && rand()%2 == 0) {
			is_inner_scope.push_back(true);
			existing_scope_ids.push_back(rand()%(int)this->scope_dictionary.size());

			actions.push_back(Action());
		} else {
			is_inner_scope.push_back(false);
			int move = rand()%3;
			actions.push_back(Action(move));

			existing_scope_ids.push_back(-1);
		}
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->id_counter << endl;

	output_file << this->scope_dictionary.size() << endl;
	for (int s_index = 0; s_index < (int)this->scope_dictionary.size(); s_index++) {
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scope_dictionary[s_index]->save(scope_save_file);
		scope_save_file.close();
	}

	ofstream scope_save_file;
	scope_save_file.open("saves/scope_root.txt");
	this->root->save(scope_save_file);
	scope_save_file.close();

	output_file << this->max_depth << endl;
}
