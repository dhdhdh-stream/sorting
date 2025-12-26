#include "solution_wrapper.h"

#include "constants.h"
#include "scope.h"
#include "solution.h"
#include "tunnel.h"

using namespace std;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->prev_solution = NULL;

	this->tunnel_iter = 0;
	this->curr_tunnel = NULL;
	this->curr_solution = new Solution();
	this->curr_solution->init(problem_type);

	this->best_solution = NULL;

	this->improvement_iter = 0;

	this->curr_experiment = NULL;
	this->best_experiment = NULL;

	this->experiment_history = NULL;

	this->has_explore = false;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */
}

SolutionWrapper::SolutionWrapper(std::string path,
								 std::string name) {
	ifstream input_file;
	input_file.open(path + name);

	string prev_solution_is_null_line;
	getline(input_file, prev_solution_is_null_line);
	bool prev_solution_is_null = stoi(prev_solution_is_null_line);
	if (prev_solution_is_null) {
		this->prev_solution = NULL;
	} else {
		this->prev_solution = new Solution();
		this->prev_solution->load(input_file);
	}

	string tunnel_iter_line;
	getline(input_file, tunnel_iter_line);
	this->tunnel_iter = stoi(tunnel_iter_line);

	string curr_tunnel_is_null_line;
	getline(input_file, curr_tunnel_is_null_line);
	bool curr_tunnel_is_null = stoi(curr_tunnel_is_null_line);
	if (curr_tunnel_is_null) {
		this->curr_tunnel = NULL;
	} else {
		this->curr_tunnel = new Tunnel(input_file);
	}

	string curr_solution_is_null_line;
	getline(input_file, curr_solution_is_null_line);
	bool curr_solution_is_null = stoi(curr_solution_is_null_line);
	if (curr_solution_is_null) {
		this->curr_solution = NULL;
	} else {
		this->curr_solution = new Solution();
		this->curr_solution->load(input_file);
	}

	string best_solution_is_null_line;
	getline(input_file, best_solution_is_null_line);
	bool best_solution_is_null = stoi(best_solution_is_null_line);
	if (best_solution_is_null) {
		this->best_solution = NULL;
	} else {
		this->best_solution = new Solution();
		this->best_solution->load(input_file);
	}

	this->improvement_iter = 0;

	this->curr_experiment = NULL;
	this->best_experiment = NULL;

	this->experiment_history = NULL;

	this->has_explore = false;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	input_file.close();
}

SolutionWrapper::~SolutionWrapper() {
	if (this->prev_solution != NULL) {
		delete this->prev_solution;
	}

	if (this->curr_tunnel != NULL) {
		delete this->curr_tunnel;
	}

	if (this->curr_solution != NULL) {
		delete this->curr_solution;
	}

	if (this->best_solution != NULL) {
		delete this->best_solution;
	}
}

bool SolutionWrapper::is_done() {
	return this->prev_solution->timestamp == -1;
}

void SolutionWrapper::clean_scopes() {
	this->prev_solution->clean_scopes();
}

void SolutionWrapper::combine(string other_path,
							  string other_name,
							  int starting_size) {
	ifstream input_file;
	input_file.open(other_path + other_name);

	Solution* other = new Solution();
	other->load(input_file);

	input_file.close();

	for (int scope_index = 1; scope_index < (int)other->scopes.size(); scope_index++) {
		this->prev_solution->scopes.push_back(other->scopes[scope_index]);

		for (int i_index = 0; i_index < starting_size; i_index++) {
			this->prev_solution->scopes[i_index]->child_scopes.push_back(other->scopes[scope_index]);
		}
	}

	other->scopes.erase(other->scopes.begin() + 1, other->scopes.end());

	delete other;

	for (int scope_index = 1; scope_index < (int)this->prev_solution->scopes.size(); scope_index++) {
		this->prev_solution->scopes[scope_index]->id = scope_index;
	}

	this->prev_solution->timestamp = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << (this->prev_solution == NULL) << endl;
	if (this->prev_solution != NULL) {
		this->prev_solution->save(output_file);
	}

	output_file << this->tunnel_iter << endl;

	output_file << (this->curr_tunnel == NULL) << endl;
	if (this->curr_tunnel != NULL) {
		this->curr_tunnel->save(output_file);
	}

	output_file << (this->curr_solution == NULL) << endl;
	if (this->curr_solution != NULL) {
		this->curr_solution->save(output_file);
	}

	output_file << (this->best_solution == NULL) << endl;
	if (this->best_solution != NULL) {
		this->best_solution->save(output_file);
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void SolutionWrapper::save_for_display(string path,
									   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	if (this->curr_solution != NULL) {
		this->curr_solution->save_for_display(output_file);
	} else {
		this->prev_solution->save_for_display(output_file);
	}

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
