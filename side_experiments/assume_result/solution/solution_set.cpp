#include "solution_set.h"

#include <fstream>
#include <iostream>

#include "solution.h"

using namespace std;

SolutionSet::SolutionSet() {
	// do nothing
}

SolutionSet::SolutionSet(SolutionSet* original) {
	this->timestamp = original->timestamp;
	this->average_score = original->average_score;

	this->best_average_score = original->best_average_score;
	this->best_timestamp = original->best_timestamp;

	this->curr_solution_index = original->curr_solution_index;
	for (int s_index = 0; s_index < (int)original->solutions.size(); s_index++) {
		this->solutions.push_back(new Solution(original->solutions[s_index]));
	}
}

SolutionSet::~SolutionSet() {
	for (int s_index = 0; s_index < (int)this->solutions.size(); s_index++) {
		delete this->solutions[s_index];
	}
}

void SolutionSet::init() {
	this->timestamp = 0;
	this->average_score = -1.0;

	this->best_average_score = -1.0;
	this->best_timestamp = 0;

	Solution* solution = new Solution();
	solution->init();
	solution->generation = 0;
	this->solutions.push_back(solution);
	this->curr_solution_index = 0;
}

void SolutionSet::load(string path,
					   string name) {
	ifstream input_file;
	input_file.open(path + "saves/" + name + ".txt");

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string average_score_line;
	getline(input_file, average_score_line);
	this->average_score = stod(average_score_line);

	string best_average_score_line;
	getline(input_file, best_average_score_line);
	this->best_average_score = stod(best_average_score_line);

	string best_timestamp_line;
	getline(input_file, best_timestamp_line);
	this->best_timestamp = stoi(best_timestamp_line);

	string num_solutions_line;
	getline(input_file, num_solutions_line);
	int num_solutions = stoi(num_solutions_line);
	for (int s_index = 0; s_index < num_solutions; s_index++) {
		Solution* solution = new Solution();
		solution->load(input_file);
		this->solutions.push_back(solution);
	}

	string curr_solution_index_line;
	getline(input_file, curr_solution_index_line);
	this->curr_solution_index = stoi(curr_solution_index_line);

	input_file.close();
}

void SolutionSet::increment() {
	#if defined(MDEBUG) && MDEBUG
	// if (rand()%4 == 0) {
	if (true) {
	#else
	// if (this->average_score > this->best_average_score) {
	if (true) {
	#endif /* MDEBUG */
		this->best_average_score = this->average_score;
		this->best_timestamp = this->timestamp;
	} else {
		if (this->timestamp >= this->best_timestamp + STALL_BEFORE_NEW) {
			bool is_merge = false;
			int match_generation;
			if (this->solutions.size() >= MERGE_SIZE) {
				is_merge = true;
				match_generation = this->solutions.back()->generation;
				for (int s_index = 0; s_index < MERGE_SIZE-1; s_index++) {
					if (this->solutions[this->solutions.size() - MERGE_SIZE + s_index]->generation != match_generation) {
						is_merge = false;
						break;
					}
				}
			}

			if (is_merge) {
				/**
				 * - TODO: instead, try multiple runs
				 *   - don't delete previous solutions, but use to build multiple new
				 * 
				 * - TODO: clean unused scopes
				 */

				Solution* solution = new Solution();
				solution->init();
				for (int s_index = 0; s_index < MERGE_SIZE; s_index++) {
					solution->merge_and_delete(this->solutions[this->solutions.size() - MERGE_SIZE + s_index]);
				}

				this->solutions.erase(this->solutions.end() - MERGE_SIZE, this->solutions.end());

				this->curr_solution_index = (int)this->solutions.size();

				solution->generation = match_generation+1;
				this->solutions.push_back(solution);
			} else {
				this->curr_solution_index = (int)this->solutions.size();

				Solution* solution = new Solution();
				solution->init();
				solution->generation = 0;
				this->solutions.push_back(solution);
			}

			this->average_score = -1.0;

			this->best_average_score = -1.0;
			this->best_timestamp = this->timestamp;
		}
	}
}

void SolutionSet::save(string path,
					   string name) {
	ofstream output_file;
	output_file.open(path + "saves/" + name + "_temp.txt");

	output_file << this->timestamp << endl;
	output_file << this->average_score << endl;

	output_file << this->best_average_score << endl;
	output_file << this->best_timestamp << endl;

	output_file << this->solutions.size() << endl;
	for (int s_index = 0; s_index < (int)this->solutions.size(); s_index++) {
		this->solutions[s_index]->save(output_file);
	}

	output_file << this->curr_solution_index << endl;

	output_file.close();

	string oldname = path + "saves/" + name + "_temp.txt";
	string newname = path + "saves/" + name + ".txt";
	rename(oldname.c_str(), newname.c_str());
}
