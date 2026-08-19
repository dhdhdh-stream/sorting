#include "solution_wrapper.h"

#include "constants.h"
#include "predict_network.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"
#include "transition_network.h"

using namespace std;

SolutionWrapper::SolutionWrapper(ProblemType* problem_type) {
	this->solution = new Solution();
	this->solution->init(problem_type);

	this->best_solution = new Solution(this->solution);

	this->iters_since_update = 0;

	this->train_iter_index = 0;

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

	this->best_solution = new Solution();
	this->best_solution->load(input_file);

	this->iters_since_update = 0;

	this->train_iter_index = 0;

	#if defined(MDEBUG) && MDEBUG
	this->run_index = 0;
	#endif /* MDEBUG */

	input_file.close();
}

SolutionWrapper::~SolutionWrapper() {
	delete this->solution;

	delete this->best_solution;

	for (int h_index = 0; h_index < (int)this->train_scope_histories.size(); h_index++) {
		delete this->train_scope_histories[h_index];
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
							  int starting_num_scopes) {
	ifstream input_file;
	input_file.open(other_path + other_name);

	Solution* other = new Solution();
	other->load(input_file);

	input_file.close();

	for (int o_index = 0; o_index < (int)other->scopes.size(); o_index++) {
		this->solution->scopes.push_back(other->scopes[o_index]);

		for (int s_index = 0; s_index < starting_num_scopes; s_index++) {
			Scope* scope = this->solution->scopes[s_index];

			scope->child_scopes.push_back(other->scopes[o_index]);

			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = scope;
			new_scope_node->id = scope->node_counter;
			scope->node_counter++;
			scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->is_generic = true;

			new_scope_node->in_network = new TransitionNetwork(scope->num_states,
															   other->scopes[o_index]->num_states);

			new_scope_node->scope = other->scopes[o_index];

			new_scope_node->out_network = new TransitionNetwork(other->scopes[o_index]->num_states,
																scope->num_states);

			new_scope_node->predict_network = new PredictNetwork(scope->num_states);

			new_scope_node->next_node_id = -1;
			new_scope_node->next_node = NULL;

			scope->generic_scope_nodes.push_back(new_scope_node);
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

	this->best_solution->save(output_file);

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
