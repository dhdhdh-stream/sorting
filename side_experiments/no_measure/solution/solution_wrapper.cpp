#include "solution_wrapper.h"

#include "constants.h"
#include "scope.h"
#include "solution.h"

using namespace std;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->solution = new Solution();
	this->solution->init(problem_type);

	this->iter = 0;

	this->history_index = 0;

	this->experiment_iter = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

SolutionWrapper::SolutionWrapper(std::string path,
								 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	this->solution = new Solution();
	this->solution->load(input_file);

	this->iter = 0;

	this->history_index = 0;

	this->experiment_iter = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	input_file.close();
}

SolutionWrapper::~SolutionWrapper() {
	delete this->solution;
}

bool SolutionWrapper::is_done() {
	return this->solution->timestamp == -1;
}

void SolutionWrapper::clean_scopes() {
	this->solution->clean_scopes();
}

void SolutionWrapper::combine(string other_path,
							  string other_name,
							  int starting_num_scopes) {
	ifstream input_file;
	input_file.open(other_path + other_name);

	Solution* other = new Solution();
	other->load(input_file);

	input_file.close();

	for (int o_index = 0; o_index < (int)other->scopes.size(); o_index++) {
		this->solution->scopes.push_back(other->scopes[o_index]);

		for (int s_index = 0; s_index < starting_num_scopes; s_index++) {
			this->solution->scopes[s_index]->child_scopes.push_back(other->scopes[o_index]);
		}
	}

	other->scopes.clear();

	delete other;

	for (int scope_index = 0; scope_index < (int)this->solution->scopes.size(); scope_index++) {
		this->solution->scopes[scope_index]->id = scope_index;
	}

	this->solution->timestamp = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void SolutionWrapper::save_for_display(string path,
									   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
