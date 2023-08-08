#include "solution.h"

#include "action_node.h"
#include "class_definition.h"
#include "family_definition.h"
#include "scope.h"
#include "score_network.h"

using namespace std;

Solution::Solution() {
	this->average_score = 0.0;
	this->score_variance = 0.0;
	this->average_misguess = 0.0;
	this->misguess_variance = 0.0;
	this->misguess_standard_deviation = 0.0;

	this->scopes.push_back(new Scope(0,
									 0,
									 vector<bool>(),
									 vector<int>(),
									 vector<int>(),
									 false,
									 NULL,
									 NULL,
									 NULL,
									 NULL));
	this->scopes[0]->nodes.push_back(new ActionNode(this->scopes[0],
													0,
													vector<int>(),
													vector<StateNetwork*>(),
													new ScoreNetwork(0,
																	 0,
																	 20),
													-1));

	this->max_depth = 1;
	this->depth_limit = 11;
}

Solution::Solution(ifstream& input_file) {
	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string score_variance_line;
	getline(input_file, score_variance_line);
	this->score_variance = stod(score_variance_line);

	string average_misguess_line;
	getline(input_file, average_misguess_line);
	this->average_misguess = stod(average_misguess_line);

	string misguess_variance_line;
	getline(input_file, misguess_variance_line);
	this->misguess_variance = stod(misguess_variance_line);

	string misguess_standard_deviation_line;
	getline(input_file, misguess_standard_deviation_line);
	this->misguess_standard_deviation = stod(misguess_standard_deviation_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);
	for (int s_index = 0; s_index < num_scopes; s_index++) {
		ifstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes.push_back(new Scope(scope_save_file, s_index));
		scope_save_file.close();
	}

	string num_families_line;
	getline(input_file, num_families_line);
	int num_families = stoi(num_families_line);
	for (int f_index = 0; f_index < num_families; f_index++) {
		this->families.push_back(new FamilyDefinition(input_file, f_index));
	}

	string num_classes_line;
	getline(input_file, num_classes_line);
	int num_classes = stoi(num_classes_line);
	for (int c_index = 0; c_index < num_classes; c_index++) {
		this->classes.push_back(new ClassDefinition(input_file, c_index));
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

	for (int f_index = 0; f_index < (int)this->families.size(); f_index++) {
		delete this->families[f_index];
	}

	for (int c_index = 0; c_index < (int)this->classes.size(); c_index++) {
		delete this->classes[c_index];
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->average_score << endl;
	output_file << this->score_variance << endl;
	output_file << this->average_misguess << endl;
	output_file << this->misguess_variance << endl;
	output_file << this->misguess_standard_deviation << endl;

	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		ofstream scope_save_file;
		scope_save_file.open("saves/scope_" + to_string(s_index) + ".txt");
		this->scopes[s_index]->save(scope_save_file);
		scope_save_file.close();
	}

	output_file << this->families.size() << endl;
	for (int f_index = 0; f_index < (int)this->families.size(); f_index++) {
		this->families[f_index]->save(output_file);
	}

	output_file << this->classes.size() << endl;
	for (int c_index = 0; c_index < (int)this->classes.size(); c_index++) {
		this->classes[c_index]->save(output_file);
	}

	output_file << this->max_depth << endl;
}

void Solution::save_for_display(ofstream& output_file) {

}
