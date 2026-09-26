#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "noop_node.h"
#include "obs_network.h"
#include "predict_network.h"
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

	this->num_obs = original->num_obs;
	this->num_actions = original->num_actions;

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
	this->cycle_index = original->cycle_index;
	this->scope_index = original->scope_index;
	this->iter_index = original->iter_index;

	for (int a_index = 0; a_index < (int)original->generic_action_nodes.size(); a_index++) {
		ActionNode* action_node = new ActionNode();
		action_node->parent = NULL;
		action_node->id = -1;
		action_node->copy_from(original->generic_action_nodes[a_index]);
		action_node->link(this);
		this->generic_action_nodes.push_back(action_node);
	}

	for (int s_index = 0; s_index < (int)original->generic_scope_nodes.size(); s_index++) {
		ScopeNode* scope_node = new ScopeNode();
		scope_node->parent = NULL;
		scope_node->id = -1;
		scope_node->copy_from(original->generic_scope_nodes[s_index],
							  this);
		scope_node->link(this);
		this->generic_scope_nodes.push_back(scope_node);
	}

	this->max_val = original->max_val;
	this->min_val = original->min_val;
	this->score_network_max_val = original->score_network_max_val;
	this->score_network_min_val = original->score_network_min_val;

	this->improvement_history = original->improvement_history;
	this->change_history = original->change_history;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
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

	this->num_obs = problem_type->num_obs();
	this->num_actions = problem_type->num_possible_actions();

	Scope* new_scope = new Scope();
	new_scope->id = this->scopes.size();
	new_scope->node_counter = 0;
	this->scopes.push_back(new_scope);

	NoopNode* start_node = new NoopNode();
	start_node->parent = new_scope;
	start_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	new_scope->nodes[start_node->id] = start_node;

	start_node->next_node_id = -1;
	start_node->next_node = NULL;

	new_scope->last_scores = vector<list<double>>(TRAIN_NEW_NUM_DATAPOINTS.size());

	this->starting_scope = new_scope;
	this->cycle_index = -1;
	this->scope_index = 0;
	this->iter_index = 0;

	for (int a_index = 0; a_index < this->num_actions; a_index++) {
		ActionNode* new_action_node = new ActionNode();
		new_action_node->parent = NULL;
		new_action_node->id = -1;

		new_action_node->is_generic = true;
		new_action_node->action = a_index;

		new_action_node->obs_network = new ObsNetwork(NUM_STATES,
													  this->num_obs);

		new_action_node->predict_network = new PredictNetwork(NUM_STATES);

		new_action_node->next_node_id = -1;
		new_action_node->next_node = NULL;

		this->generic_action_nodes.push_back(new_action_node);
	}

	this->max_val = numeric_limits<double>::min();
	this->min_val = numeric_limits<double>::max();
	this->score_network_max_val = numeric_limits<double>::max();
	this->score_network_min_val = numeric_limits<double>::min();
}

void Solution::load(ifstream& input_file) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string num_obs_line;
	getline(input_file, num_obs_line);
	this->num_obs = stoi(num_obs_line);

	string num_actions_line;
	getline(input_file, num_actions_line);
	this->num_actions = stoi(num_actions_line);

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

	string cycle_index_line;
	getline(input_file, cycle_index_line);
	this->cycle_index = stoi(cycle_index_line);

	string scope_index_line;
	getline(input_file, scope_index_line);
	this->scope_index = stoi(scope_index_line);

	string iter_index_line;
	getline(input_file, iter_index_line);
	this->iter_index = stoi(iter_index_line);

	for (int a_index = 0; a_index < this->num_actions; a_index++) {
		ActionNode* action_node = new ActionNode();
		action_node->parent = NULL;
		action_node->id = -1;
		action_node->load(input_file);
		action_node->link(this);
		this->generic_action_nodes.push_back(action_node);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		ScopeNode* scope_node = new ScopeNode();
		scope_node->parent = NULL;
		scope_node->id = -1;
		scope_node->load(input_file,
						 this);
		scope_node->link(this);
		this->generic_scope_nodes.push_back(scope_node);
	}

	string max_val_line;
	getline(input_file, max_val_line);
	this->max_val = stod(max_val_line);

	string min_val_line;
	getline(input_file, min_val_line);
	this->min_val = stod(min_val_line);

	this->score_network_max_val = this->max_val + (this->max_val - this->min_val)/2.0;
	this->score_network_min_val = this->min_val - (this->max_val - this->min_val)/2.0;

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

	output_file << this->num_obs << endl;
	output_file << this->num_actions << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->starting_scope->id << endl;
	output_file << this->cycle_index << endl;
	output_file << this->scope_index << endl;
	output_file << this->iter_index << endl;

	for (int a_index = 0; a_index < (int)this->generic_action_nodes.size(); a_index++) {
		this->generic_action_nodes[a_index]->save(output_file);
	}

	for (int s_index = 0; s_index < (int)this->generic_scope_nodes.size(); s_index++) {
		this->generic_scope_nodes[s_index]->save(output_file);
	}

	output_file << this->max_val << endl;
	output_file << this->min_val << endl;

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
