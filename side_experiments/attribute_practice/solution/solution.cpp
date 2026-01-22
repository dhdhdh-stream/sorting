#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "long_network.h"
#include "obs_node.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"
#include "start_node.h"

using namespace std;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_score = original->curr_score;

	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		Scope* scope = new Scope();
		scope->id = s_index;
		this->scopes.push_back(scope);
	}

	for (int s_index = 0; s_index < (int)original->scopes.size(); s_index++) {
		this->scopes[s_index]->copy_from(original->scopes[s_index],
										 this);
	}

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->link(this);
	}

	this->last_experiment_scores = original->last_experiment_scores;

	for (int h_index = 0; h_index < (int)original->existing_scope_histories.size(); h_index++) {
		this->existing_scope_histories.push_back(new ScopeHistory(original->existing_scope_histories[h_index],
																  this));
	}
	this->existing_target_val_histories = original->existing_target_val_histories;

	this->improvement_history = original->improvement_history;
	this->change_history = original->change_history;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}

	for (int h_index = 0; h_index < (int)this->existing_scope_histories.size(); h_index++) {
		delete this->existing_scope_histories[h_index];
	}

	#if defined(MDEBUG) && MDEBUG
	for (int p_index = 0; p_index < (int)this->verify_problems.size(); p_index++) {
		delete this->verify_problems[p_index];
	}
	#endif /* MDEBUG */
}

void Solution::init(ProblemType* problem_type) {
	double sum_score = 0.0;
	for (int iter_index = 0; iter_index < MEASURE_ITERS; iter_index++) {
		Problem* problem = problem_type->get_problem();
		sum_score += problem->score_result();
		delete problem;
	}

	this->timestamp = 0;
	this->curr_score = sum_score / MEASURE_ITERS;

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

	new_scope->pre_network = new LongNetwork(problem_type->num_obs());
	new_scope->post_network = new LongNetwork(problem_type->num_obs());
}

void Solution::load(ifstream& input_file) {
	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

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

	string num_experiment_scores_line;
	getline(input_file, num_experiment_scores_line);
	int num_experiment_scores = stoi(num_experiment_scores_line);
	for (int e_index = 0; e_index < num_experiment_scores; e_index++) {
		string score_line;
		getline(input_file, score_line);
		this->last_experiment_scores.push_back(stod(score_line));
	}

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

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::clean_scopes() {
	while (true) {
		bool removed_scope = false;
		for (int s_index = (int)this->scopes.size()-1; s_index >= 1; s_index--) {
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

		if (!removed_scope) {
			break;
		}
	}

	for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->id = s_index;
	}
}

void Solution::save(ofstream& output_file) {
	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->last_experiment_scores.size() << endl;
	for (list<double>::iterator it = this->last_experiment_scores.begin();
			it != this->last_experiment_scores.end(); it++) {
		output_file << *it << endl;
	}

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
