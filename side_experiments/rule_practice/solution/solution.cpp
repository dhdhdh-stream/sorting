#include "solution.h"

#include "rule.h"
#include "solution_helpers.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::~Solution() {
	for (int r_index = 0; r_index < (int)this->rules.size(); r_index++) {
		delete this->rules[r_index];
	}
}

void Solution::init() {
	rule_experiment(this->rules,
					this->average_val,
					this->top_5_percentile,
					this->top_5_percentile_average_val);
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + name);

	string num_rules_line;
	getline(input_file, num_rules_line);
	int num_rules = stoi(num_rules_line);
	for (int r_index = 0; r_index < num_rules; r_index++) {
		this->rules.push_back(new Rule(input_file));
	}

	string average_val_line;
	getline(input_file, average_val_line);
	this->average_val = stod(average_val_line);

	string top_5_percentile_line;
	getline(input_file, top_5_percentile_line);
	this->top_5_percentile = stod(top_5_percentile_line);

	string top_5_percentile_average_val_line;
	getline(input_file, top_5_percentile_average_val_line);
	this->top_5_percentile_average_val = stod(top_5_percentile_average_val_line);

	input_file.close();
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->rules.size() << endl;
	for (int r_index = 0; r_index < (int)this->rules.size(); r_index++) {
		this->rules[r_index]->save(output_file);
	}

	output_file << this->average_val << endl;
	output_file << this->top_5_percentile << endl;
	output_file << this->top_5_percentile_average_val << endl;

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
