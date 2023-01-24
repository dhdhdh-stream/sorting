#include "solution.h"

#include <random>

#include "globals.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	delete this->root;
}

void Solution::init() {
	this->id_counter = 0;

	int starting_num_inputs = 0;
	int starting_num_outputs = 0;
	int starting_sequence_length = 1;
	vector<bool> starting_is_inner_scope{false};
	vector<Scope*> starting_scopes{NULL};
	vector<Action> starting_actions{Action(ACTION_START, 0.0)};
	vector<vector<FoldNetwork*>> starting_inner_input_networks{vector<FoldNetwork*>()};
	vector<vector<int>> starting_inner_input_sizes{vector<int>()};
	vector<double> starting_scope_scale_mod{0.0};	// doesn't matter
	vector<int> starting_step_types{STEP_TYPE_STEP};
	vector<Branch*> starting_branches{NULL};
	vector<Fold*> starting_folds{NULL};
	FoldNetwork* starting_score_network = new FoldNetwork(
		1,
		0,
		vector<int>{1},
		20);
	vector<FoldNetwork*> starting_score_networks{starting_score_network};
	vector<double> starting_average_scores{0.0};
	vector<double> starting_average_misguesses{0.0};
	vector<double> starting_average_inner_scope_impacts{0.0};
	vector<double> starting_average_local_impacts{0.0};
	vector<double> starting_average_inner_branch_impacts{0.0};
	vector<bool> starting_active_compress{false};
	vector<int> starting_compress_new_sizes{0};
	vector<FoldNetwork*> starting_compress_networks{NULL};
	vector<int> starting_compress_original_sizes{1};
	bool starting_full_last = true;
	this->root = new Scope(starting_num_inputs,
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
						   starting_average_scores,
						   starting_average_misguesses,
						   starting_average_inner_scope_impacts,
						   starting_average_local_impacts,
						   starting_average_inner_branch_impacts,
						   starting_active_compress,
						   starting_compress_new_sizes,
						   starting_compress_networks,
						   starting_compress_original_sizes,
						   starting_full_last);
}

void Solution::load(ifstream& input_file) {
	string id_counter_line;
	getline(input_file, id_counter_line);
	this->id_counter = stoi(id_counter_line);

	// not needed as should always be 0
	string scope_id_line;
	getline(input_file, scope_id_line);
	int scope_id = stoi(scope_id_line);

	ifstream scope_save_file;
	scope_save_file.open("saves/scope_" + to_string(scope_id) + ".txt");
	this->root = new Scope(scope_save_file);
	scope_save_file.close();

	// remove root scope from dictionary
	this->scope_dictionary.pop_back();
}

void Solution::new_sequence(int& sequence_length,
							vector<bool>& is_existing,
							vector<Scope*>& existing_actions,
							vector<Action>& actions) {
	geometric_distribution<int> geo_dist(0.3);
	sequence_length = 1 + geo_dist(generator);
	for (int s_index = 0; s_index < sequence_length; s_index++) {
		if (this->scope_dictionary.size() > 0 && rand()%2 == 0) {
			is_existing.push_back(true);
			int rand_index = rand()%(int)this->scope_dictionary.size();
			existing_actions.push_back(this->scope_dictionary[rand_index]);

			actions.push_back(Action());
		} else {
			is_existing.push_back(false);
			if (this->action_dictionary.size() > 0 && rand()%2 == 0) {
				int rand_index = rand()%(int)this->action_dictionary.size();
				actions.push_back(this->action_dictionary[rand_index]);
			} else {
				normal_distribution<double> norm_dist(0.0, 1.0);
				double write = norm_dist(generator);

				int move = rand()%3;

				actions.push_back(Action(write, move));
			}

			existing_actions.push_back(NULL);
		}
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->id_counter << endl;

	output_file << this->root->id << endl;

	ofstream scope_save_file;
	scope_save_file.open("saves/scope_" + to_string(this->root->id) + ".txt");
	this->root->save(scope_save_file);
	scope_save_file.close();
}
