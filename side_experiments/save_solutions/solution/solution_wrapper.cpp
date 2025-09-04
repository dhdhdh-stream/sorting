#include "solution_wrapper.h"

#include <iostream>

#include "constants.h"
#include "obs_node.h"
#include "scope.h"
#include "signal.h"
#include "solution.h"
#include "start_node.h"

using namespace std;

SolutionWrapper::SolutionWrapper(int num_obs) {
	this->num_obs = num_obs;

	this->scope_counter = 0;

	this->solution = new Solution();
	{
		this->solution->timestamp = 0;
		this->solution->curr_val_average = 0.0;
		this->solution->curr_val_standard_deviation = 0.0;

		/**
		 * - even though scopes[0] will not be reused, still good to start with:
		 *   - if artificially add empty scopes, may be difficult to extend from
		 *     - and will then junk up explore
		 *   - new scopes will be created from the reusable portions anyways
		 */

		Scope* new_scope = new Scope();
		new_scope->id = this->scope_counter;
		this->scope_counter++;
		this->solution->scopes[new_scope->id] = new_scope;

		new_scope->node_counter = 0;

		StartNode* start_node = new StartNode();
		start_node->parent = new_scope;
		start_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[start_node->id] = start_node;

		ObsNode* end_node = new ObsNode();
		end_node->parent = new_scope;
		end_node->id = new_scope->node_counter;
		new_scope->node_counter++;
		new_scope->nodes[end_node->id] = end_node;

		start_node->next_node_id = end_node->id;
		start_node->next_node = end_node;

		end_node->ancestor_ids.push_back(start_node->id);

		end_node->next_node_id = -1;
		end_node->next_node = NULL;

		this->solution->clean();
	}

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

	ifstream input_file;
	input_file.open(path + name);

	string scope_counter_line;
	getline(input_file, scope_counter_line);
	this->scope_counter = stoi(scope_counter_line);

	this->solution = new Solution();
	this->solution->load(input_file);

	string num_positive_solutions_line;
	getline(input_file, num_positive_solutions_line);
	int num_positive_solutions = stoi(num_positive_solutions_line);
	for (int s_index = 0; s_index < num_positive_solutions; s_index++) {
		Solution* l_solution = new Solution();
		l_solution->load(input_file);
		this->positive_solutions.push_back(l_solution);
	}

	string num_signals_line;
	getline(input_file, num_signals_line);
	int num_signals = stoi(num_signals_line);
	for (int s_index = 0; s_index < num_signals; s_index++) {
		string scope_id_line;
		getline(input_file, scope_id_line);
		int scope_id = stoi(scope_id_line);

		Signal* signal = new Signal(input_file);

		this->signals[scope_id] = signal;
	}

	input_file.close();

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

	for (int s_index = 0; s_index < (int)this->positive_solutions.size(); s_index++) {
		delete this->positive_solutions[s_index];
	}

	for (map<int, Signal*>::iterator it = this->signals.begin();
			it != this->signals.end(); it++) {
		delete it->second;
	}
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

	map<int, Scope*> existing_scopes = this->solution->scopes;

	for (map<int, Scope*>::iterator new_it = other->scopes.begin();
			new_it != other->scopes.end(); new_it++) {
		new_it->second->id = this->scope_counter;
		this->scope_counter++;

		this->solution->scopes[new_it->second->id] = new_it->second;

		for (map<int, Scope*>::iterator existing_it = existing_scopes.begin();
				existing_it != existing_scopes.end(); existing_it++) {
			existing_it->second->child_scopes.push_back(new_it->second);
		}
	}
	other->scopes.clear();

	delete other;

	this->solution->timestamp = 0;
}

void SolutionWrapper::save(string path,
						   string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->scope_counter << endl;

	this->solution->save(output_file);

	cout << "this->positive_solutions.size(): " << this->positive_solutions.size() << endl;

	output_file << this->positive_solutions.size() << endl;
	for (int s_index = 0; s_index < (int)this->positive_solutions.size(); s_index++) {
		this->positive_solutions[s_index]->save(output_file);
	}

	output_file << this->signals.size() << endl;
	for (map<int, Signal*>::iterator it = this->signals.begin();
			it != this->signals.end(); it++) {
		output_file << it->first << endl;
		it->second->save(output_file);
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

	this->solution->save_for_display(output_file);

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}
