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
#include "transition_network.h"

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

	this->outer_scope = this->scopes[original->outer_scope->id];
	this->top_scope = this->scopes[original->top_scope->id];
	this->top_scope_num_improvements = original->top_scope_num_improvements;

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

	{
		Scope* top_scope = new Scope();
		top_scope->id = this->scopes.size();
		top_scope->node_counter = 0;
		this->scopes.push_back(top_scope);

		NoopNode* start_node = new NoopNode();
		start_node->parent = top_scope;
		start_node->id = top_scope->node_counter;
		top_scope->node_counter++;
		top_scope->nodes[start_node->id] = start_node;

		start_node->next_node_id = -1;
		start_node->next_node = NULL;

		top_scope->obs_network = new ObsNetwork(NUM_STATES,
												this->num_obs);

		top_scope->score_network = new ScoreNetwork(NUM_STATES);

		for (int a_index = 0; a_index < this->num_actions; a_index++) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = top_scope;
			new_action_node->id = top_scope->node_counter;
			top_scope->node_counter++;
			top_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->is_generic = true;
			new_action_node->action = a_index;

			new_action_node->obs_network = new ObsNetwork(NUM_STATES,
														  this->num_obs);

			new_action_node->predict_network = new PredictNetwork(NUM_STATES);

			new_action_node->next_node_id = -1;
			new_action_node->next_node = NULL;

			top_scope->generic_action_nodes.push_back(new_action_node);
		}

		top_scope->train_new_last_scores = vector<list<double>>(TRAIN_NEW_NUM_DATAPOINTS.size());

		this->top_scope = top_scope;
		this->top_scope_num_improvements = 0;
	}

	{
		Scope* outer_scope = new Scope();
		outer_scope->id = this->scopes.size();
		outer_scope->node_counter = 0;
		this->scopes.push_back(outer_scope);

		outer_scope->child_scopes.push_back(this->top_scope);

		NoopNode* start_node = new NoopNode();
		start_node->parent = outer_scope;
		start_node->id = outer_scope->node_counter;
		outer_scope->node_counter++;
		outer_scope->nodes[start_node->id] = start_node;

		ScopeNode* scope_node = new ScopeNode();
		scope_node->parent = outer_scope;
		scope_node->id = outer_scope->node_counter;
		outer_scope->node_counter++;
		outer_scope->nodes[scope_node->id] = scope_node;

		scope_node->is_generic = false;
		scope_node->scope = this->top_scope;

		scope_node->out_network = new TransitionNetwork(NUM_STATES,
														NUM_STATES);

		scope_node->predict_network = new PredictNetwork(NUM_STATES);

		NoopNode* end_node = new NoopNode();
		end_node->parent = outer_scope;
		end_node->id = outer_scope->node_counter;
		outer_scope->node_counter++;
		outer_scope->nodes[end_node->id] = end_node;

		start_node->next_node_id = scope_node->id;
		start_node->next_node = scope_node;

		scope_node->ancestor_ids.push_back(start_node->id);

		scope_node->next_node_id = end_node->id;
		scope_node->next_node = end_node;

		end_node->ancestor_ids.push_back(scope_node->id);

		end_node->next_node_id = -1;
		end_node->next_node = NULL;

		outer_scope->obs_network = new ObsNetwork(NUM_STATES,
												  this->num_obs);

		outer_scope->score_network = new ScoreNetwork(NUM_STATES);

		for (int a_index = 0; a_index < this->num_actions; a_index++) {
			ActionNode* new_action_node = new ActionNode();
			new_action_node->parent = outer_scope;
			new_action_node->id = outer_scope->node_counter;
			outer_scope->node_counter++;
			outer_scope->nodes[new_action_node->id] = new_action_node;

			new_action_node->is_generic = true;
			new_action_node->action = a_index;

			new_action_node->obs_network = new ObsNetwork(NUM_STATES,
														  this->num_obs);

			new_action_node->predict_network = new PredictNetwork(NUM_STATES);

			new_action_node->next_node_id = -1;
			new_action_node->next_node = NULL;

			outer_scope->generic_action_nodes.push_back(new_action_node);
		}

		{
			ScopeNode* new_scope_node = new ScopeNode();
			new_scope_node->parent = outer_scope;
			new_scope_node->id = outer_scope->node_counter;
			outer_scope->node_counter++;
			outer_scope->nodes[new_scope_node->id] = new_scope_node;

			new_scope_node->is_generic = true;

			new_scope_node->scope = this->top_scope;

			new_scope_node->out_network = new TransitionNetwork(NUM_STATES,
																NUM_STATES);

			new_scope_node->predict_network = new PredictNetwork(NUM_STATES);

			new_scope_node->next_node_id = -1;
			new_scope_node->next_node = NULL;

			outer_scope->generic_scope_nodes.push_back(new_scope_node);
		}

		outer_scope->train_new_last_scores = vector<list<double>>(TRAIN_NEW_NUM_DATAPOINTS.size());

		this->outer_scope = outer_scope;
	}
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

	string outer_scope_id_line;
	getline(input_file, outer_scope_id_line);
	this->outer_scope = this->scopes[stoi(outer_scope_id_line)];

	string top_scope_id_line;
	getline(input_file, top_scope_id_line);
	this->top_scope = this->scopes[stoi(top_scope_id_line)];

	string top_scope_num_improvements_line;
	getline(input_file, top_scope_num_improvements_line);
	this->top_scope_num_improvements = stoi(top_scope_num_improvements_line);

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
			if (s_index != this->outer_scope->id) {
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

	output_file << this->outer_scope->id << endl;
	output_file << this->top_scope->id << endl;
	output_file << this->top_scope_num_improvements << endl;

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
