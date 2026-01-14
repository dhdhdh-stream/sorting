#include "solution_wrapper.h"

#include "constants.h"
#include "scope.h"
#include "solution.h"
#include "tunnel.h"

using namespace std;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->solution = new Solution();
	this->solution->init(problem_type);

	this->solution_snapshots.push_back(new Solution(this->solution));
	this->num_resets.push_back(0);
	this->reset_count = 0;

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

	string num_candidates_line;
	getline(input_file, num_candidates_line);
	int num_candidates = stoi(num_candidates_line);
	for (int c_index = 0; c_index < num_candidates; c_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		Tunnel* tunnel = new Tunnel(input_file);

		this->candidates.push_back({scope_id, tunnel});
	}

	string num_snapshots_line;
	getline(input_file, num_snapshots_line);
	int num_snapshots = stoi(num_snapshots_line);
	for (int s_index = 0; s_index < num_snapshots; s_index++) {
		Solution* snapshot = new Solution();
		snapshot->load(input_file);
		this->solution_snapshots.push_back(snapshot);

		string num_resets_line;
		getline(input_file, num_resets_line);
		this->num_resets.push_back(stoi(num_resets_line));
	}

	string reset_count_line;
	getline(input_file, reset_count_line);
	this->reset_count = stoi(reset_count_line);

	this->curr_experiment = NULL;

	this->experiment_history = NULL;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	input_file.close();
}

SolutionWrapper::~SolutionWrapper() {
	delete this->solution;

	for (int c_index = 0; c_index < (int)this->candidates.size(); c_index++) {
		delete this->candidates[c_index].second;
	}
}

bool SolutionWrapper::is_done() {
	return this->solution->timestamp == -1;
}

void SolutionWrapper::clean_scopes() {
	this->solution->clean_scopes();
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
		this->solution->scopes.push_back(other->scopes[scope_index]);

		for (int i_index = 0; i_index < starting_size; i_index++) {
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
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	this->solution->save(output_file);

	output_file << this->candidates.size() << endl;
	for (int c_index = 0; c_index < (int)this->candidates.size(); c_index++) {
		output_file << this->candidates[c_index].first << endl;

		this->candidates[c_index].second->save(output_file);
	}

	output_file << this->solution_snapshots.size() << endl;
	for (int s_index = 0; s_index < (int)this->solution_snapshots.size(); s_index++) {
		this->solution_snapshots[s_index]->save(output_file);

		output_file << this->num_resets[s_index] << endl;
	}

	output_file << this->reset_count << endl;

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
