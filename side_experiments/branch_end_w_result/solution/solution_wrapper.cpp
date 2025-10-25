#include "solution_wrapper.h"

#include "scope.h"
#include "solution.h"

using namespace std;

SolutionWrapper::SolutionWrapper() {
	this->solution = new Solution();
	this->solution->init();

	this->improvement_iter = 0;

	this->curr_branch_experiment = NULL;
	this->best_branch_experiment = NULL;

	this->experiment_history = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

SolutionWrapper::SolutionWrapper(std::string path,
								 std::string name) {
	this->solution = new Solution();
	this->solution->load(path, name);

	this->improvement_iter = 0;

	this->curr_branch_experiment = NULL;
	this->best_branch_experiment = NULL;

	this->experiment_history = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
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
	Solution* other = new Solution();
	other->load(other_path, other_name);

	{
		this->solution->external_scopes.push_back(other->scopes[0]);
		this->solution->external_is_root.push_back(true);
	}
	for (int scope_index = 1; scope_index < (int)other->scopes.size(); scope_index++) {
		this->solution->external_scopes.push_back(other->scopes[scope_index]);
		this->solution->external_is_root.push_back(false);
	}
	other->scopes.clear();

	delete other;

	for (int scope_index = 0; scope_index < (int)this->solution->external_scopes.size(); scope_index++) {
		this->solution->external_scopes[scope_index]->id = scope_index;
		this->solution->external_scopes[scope_index]->is_external = true;
	}

	this->solution->timestamp = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	this->solution->save(path, name);
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
