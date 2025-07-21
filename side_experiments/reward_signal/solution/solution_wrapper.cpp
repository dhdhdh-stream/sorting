#include "solution_wrapper.h"

#include "scope.h"
#include "solution.h"

using namespace std;

SolutionWrapper::SolutionWrapper(int num_obs) {
	this->num_obs = num_obs;

	this->solution = new Solution();
	this->solution->init();

	this->curr_experiment = NULL;
	this->best_experiment = NULL;
	this->improvement_iter = 0;

	this->experiment_overall_history = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

SolutionWrapper::SolutionWrapper(int num_obs,
								 std::string path,
								 std::string name) {
	this->num_obs = num_obs;

	this->solution = new Solution();
	this->solution->load(path, name);

	this->curr_experiment = NULL;
	this->best_experiment = NULL;
	this->improvement_iter = 0;

	this->experiment_overall_history = NULL;

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

	int existing_num_scopes = (int)this->solution->scopes.size();

	for (int scope_index = 1; scope_index < (int)other->scopes.size(); scope_index++) {
		this->solution->scopes.push_back(other->scopes[scope_index]);

		for (int i_index = 0; i_index < existing_num_scopes; i_index++) {
			this->solution->scopes[i_index]->child_scopes.push_back(other->scopes[scope_index]);
		}
	}

	other->scopes.erase(other->scopes.begin() + 1, other->scopes.end());

	delete other;

	for (int scope_index = 1; scope_index < (int)this->solution->scopes.size(); scope_index++) {
		this->solution->scopes[scope_index]->id = scope_index;
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
