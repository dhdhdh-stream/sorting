#include "solution_wrapper.h"

#include "constants.h"
#include "scope.h"
#include "solution.h"

using namespace std;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->solution = new Solution();
	this->solution->init(problem_type);

	this->curr_experiment = NULL;

	this->experiment_history = NULL;

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

	this->curr_experiment = NULL;

	this->experiment_history = NULL;

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
							  string other_name) {
	ifstream input_file;
	input_file.open(other_path + other_name);

	Solution* other = new Solution();
	other->load(input_file);

	input_file.close();

	this->solution->outer_root_scope_ids.push_back(this->solution->outer_scopes.size());

	for (int o_index = 0; o_index < (int)other->scopes.size(); o_index++) {
		this->solution->outer_scopes.push_back(other->scopes[o_index]);

		for (int s_index = 0; s_index < (int)this->solution->scopes.size(); s_index++) {
			this->solution->scopes[s_index]->child_scopes.push_back(other->scopes[o_index]);
		}
	}

	other->scopes.clear();

	delete other;

	for (int scope_index = 0; scope_index < (int)this->solution->outer_scopes.size(); scope_index++) {
		this->solution->outer_scopes[scope_index]->is_outer = true;
		this->solution->outer_scopes[scope_index]->id = scope_index;
	}

	this->solution->last_scores.clear();

	this->solution->state = SOLUTION_STATE_OUTER;
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
