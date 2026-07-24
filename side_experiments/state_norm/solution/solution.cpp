#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_network.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "obs_network.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "solution_helpers.h"

using namespace std;

#if defined(MDEBUG) && MDEBUG
const int INIT_MEASURE_ITERS = 10;
#else
const int INIT_MEASURE_ITERS = 4000;
#endif /* MDEBUG */

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_score = original->curr_score;

	this->curr_num_resets = original->curr_num_resets;

	this->num_obs = original->num_obs;

	this->num_states = original->num_states;

	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	this->starting_scope = this->scopes[original->starting_scope->id];
	this->starting_num_improvements = original->starting_num_improvements;

	for (int a_index = 0; a_index < (int)original->generic_action_networks.size(); a_index++) {
		this->generic_action_networks.push_back(new ActionNetwork(original->generic_action_networks[a_index]));
	}
	this->generic_obs_network = new ObsNetwork(original->generic_obs_network);

	this->average_max_update = original->average_max_update;

	this->improvement_history = original->improvement_history;
	this->change_history = original->change_history;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}

	for (int a_index = 0; a_index < (int)this->generic_action_networks.size(); a_index++) {
		delete this->generic_action_networks[a_index];
	}
	delete this->generic_obs_network;
}

void Solution::init(ProblemType* problem_type) {
	double sum_score = 0.0;
	for (int iter_index = 0; iter_index < INIT_MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();
		sum_score += problem->score_result();
		delete problem;
	}

	this->timestamp = 0;
	this->curr_score = sum_score / INIT_MEASURE_ITERS;

	this->curr_num_resets = 0;

	this->num_obs = problem_type->num_obs();

	this->num_states = 0;

	/**
	 * - even though scopes[0] will not be reused, still good to start with:
	 *   - if artificially add empty scopes, may be difficult to extend from
	 *     - and will then junk up explore
	 *   - new scopes will be created from the reusable portions anyways
	 */

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->node_counter = 0;
	this->scopes.push_back(new_scope);
	new_scope->start_obs_network = new ObsNetwork(this->num_states,
												  this->num_obs);

	NoopNode* start_node = new NoopNode();
	start_node->parent = new_scope;
	start_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[start_node->id] = start_node;

	start_node->next_node_id = -1;
	start_node->next_node = NULL;

	this->starting_scope = new_scope;
	this->starting_num_improvements = 0;

	int num_actions = problem_type->num_possible_actions();
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->generic_action_networks.push_back(new ActionNetwork(this->num_states));
	}
	this->generic_obs_network = new ObsNetwork(this->num_states,
											   this->num_obs);

	this->average_max_update = 0.0;
}

void Solution::load(ifstream& input_file) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string curr_num_resets_line;
	getline(input_file, curr_num_resets_line);
	this->curr_num_resets = stoi(curr_num_resets_line);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_states_line;
	getline(input_file, num_states_line);
	this->num_states = stoi(num_states_line);

	string num_scopes_line;
	getline(input_file, num_scopes_line);
	int num_scopes = stoi(num_scopes_line);

	for (int s_index = 0; s_index < num_scopes; s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->load(input_file,
									this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	string starting_scope_id_line;
	getline(input_file, starting_scope_id_line);
	this->starting_scope = this->scopes[stoi(starting_scope_id_line)];

	string starting_num_improvements_line;
	getline(input_file, starting_num_improvements_line);
	this->starting_num_improvements = stoi(starting_num_improvements_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	int num_actions = stoi(num_actions_line);
	for (int a_index = 0; a_index < num_actions; a_index++) {
		this->generic_action_networks.push_back(new ActionNetwork(input_file));
	}
	this->generic_obs_network = new ObsNetwork(input_file);

	string average_max_update_line;
	getline(input_file, average_max_update_line);
	this->average_max_update = stod(average_max_update_line);

	string history_size_line;
	getline(input_file, history_size_line);
	int history_size = stoi(history_size_line);
	for (int h_index = 0; h_index < history_size; h_index++) {
		string improvement_line;
		getline(input_file, improvement_line);
		this->improvement_history.push_back(stod(improvement_line));

		string change_line;
		getline(input_file, change_line);
		this->change_history.push_back(change_line);
	}
}

void Solution::clean_scopes() {
	while (true) {
		bool removed_scope = false;
		for (int s_index = (int)this->scopes.size()-1; s_index >= 0; s_index--) {
			if (s_index != this->starting_scope->id) {
				bool still_used = false;
				for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
					if (s_index != is_index) {
						for (map<int, AbstractNode*>::iterator it = this->scopes[is_index]->nodes.begin();
								it != this->scopes[is_index]->nodes.end(); it++) {
							switch (it->second->type) {
							case NODE_TYPE_SCOPE:
								{
									ScopeNode* scope_node = (ScopeNode*)it->second;
									if (scope_node->scope == this->scopes[s_index]) {
										still_used = true;
										break;
									}
								}
								break;
							}
						}
					}

					if (still_used) {
						break;
					}
				}

				if (!still_used) {
					removed_scope = true;

					for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
						for (int c_index = 0; c_index < (int)this->scopes[is_index]->child_scopes.size(); c_index++) {
							if (this->scopes[is_index]->child_scopes[c_index] == this->scopes[s_index]) {
								this->scopes[is_index]->child_scopes.erase(this->scopes[is_index]->child_scopes.begin() + c_index);
								break;
							}
						}
					}

					delete this->scopes[s_index];
					this->scopes.erase(this->scopes.begin() + s_index);
				}
			}
		}

		if (!removed_scope) {
			break;
		}

		for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
			this->scopes[s_index]->id = s_index;
		}
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->curr_num_resets << endl;

	output_file << this->num_obs << endl;

	output_file << this->num_states << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->starting_scope->id << endl;
	output_file << this->starting_num_improvements << endl;

	output_file << this->generic_action_networks.size() << endl;
	for (int a_index = 0; a_index < (int)this->generic_action_networks.size(); a_index++) {
		this->generic_action_networks[a_index]->save(output_file);
	}
	this->generic_obs_network->save(output_file);

	output_file << this->average_max_update << endl;

	output_file << this->improvement_history.size() << endl;
	for (int h_index = 0; h_index < (int)this->improvement_history.size(); h_index++) {
		output_file << this->improvement_history[h_index] << endl;
		output_file << this->change_history[h_index] << endl;
	}
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
