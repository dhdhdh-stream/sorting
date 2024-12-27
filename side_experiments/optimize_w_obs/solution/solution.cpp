#include "solution.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "globals.h"
#include "potential_commit.h"
#include "problem.h"
#include "scope.h"
#include "scope_node.h"
#include "solution_helpers.h"

using namespace std;

const double STARTING_TIME_PENALTY = 0.001;

Solution::Solution() {
	// do nothing
}

Solution::Solution(Solution* original) {
	this->timestamp = original->timestamp;
	this->curr_score = original->curr_score;

	this->curr_true_score = original->curr_true_score;
	this->best_true_score = original->best_true_score;
	this->best_true_score_timestamp = original->best_true_score_timestamp;
	this->curr_time_penalty = original->curr_time_penalty;

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

	this->num_existing_scopes = original->num_existing_scopes;

	this->was_commit = false;
}

Solution::~Solution() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		delete this->scopes[s_index];
	}
}

void Solution::init() {
	this->timestamp = 0;
	this->curr_score = -1.0;

	this->curr_true_score = -1.0;
	this->best_true_score = -1.0;
	this->best_true_score_timestamp = 0;
	this->curr_time_penalty = STARTING_TIME_PENALTY;

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

	ActionNode* starting_noop_node = new ActionNode();
	starting_noop_node->parent = new_scope;
	starting_noop_node->id = new_scope->node_counter;
	new_scope->node_counter++;
	starting_noop_node->action = Action(ACTION_NOOP);
	starting_noop_node->next_node_id = -1;
	starting_noop_node->next_node = NULL;
	starting_noop_node->average_instances_per_run = 1.0;
	new_scope->nodes[starting_noop_node->id] = starting_noop_node;

	this->num_existing_scopes = 0;

	this->was_commit = false;
}

void Solution::load(string path,
					string name) {
	ifstream input_file;
	input_file.open(path + name);

	string timestamp_line;
	getline(input_file, timestamp_line);
	this->timestamp = stoi(timestamp_line);

	string curr_score_line;
	getline(input_file, curr_score_line);
	this->curr_score = stod(curr_score_line);

	string curr_true_score_line;
	getline(input_file, curr_true_score_line);
	this->curr_true_score = stod(curr_true_score_line);

	string best_true_score_line;
	getline(input_file, best_true_score_line);
	this->best_true_score = stod(best_true_score_line);

	string best_true_score_timestamp_line;
	getline(input_file, best_true_score_timestamp_line);
	this->best_true_score_timestamp = stoi(best_true_score_timestamp_line);

	string curr_time_penalty_line;
	getline(input_file, curr_time_penalty_line);
	this->curr_time_penalty = stod(curr_time_penalty_line);

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

	string num_existing_scopes_line;
	getline(input_file, num_existing_scopes_line);
	this->num_existing_scopes = stoi(num_existing_scopes_line);

	string was_commit_line;
	getline(input_file, was_commit_line);
	this->was_commit = stoi(was_commit_line);

	input_file.close();
}

#if defined(MDEBUG) && MDEBUG
void Solution::clear_verify() {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clear_verify();
	}

	this->verify_problems.clear();
}
#endif /* MDEBUG */

void Solution::clean_inputs(Scope* scope,
							int node_id) {
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->clean_inputs(scope,
											node_id);
	}
}

void Solution::clean_scopes() {
	if (this->timestamp > MAINTAIN_ITERS) {
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
				for (int is_index = 0; is_index < (int)this->scopes.size(); is_index++) {
					this->scopes[is_index]->clean_inputs(this->scopes[s_index]);

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

		for (int s_index = 1; s_index < (int)this->scopes.size(); s_index++) {
			this->scopes[s_index]->id = s_index;
		}
	}
}

void Solution::commit() {
	while (true) {
		run_index++;

		Problem* problem = problem_type->get_problem();

		RunHelper run_helper;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> commit_gather_context;
		int node_count = 0;
		AbstractNode* potential_node_context;
		bool potential_is_branch;
		this->scopes[0]->commit_gather_activate(
				problem,
				commit_gather_context,
				run_helper,
				node_count,
				potential_node_context,
				potential_is_branch);

		double existing_target_val = problem->score_result();
		existing_target_val -= 0.05 * run_helper.num_actions * this->curr_time_penalty;
		existing_target_val -= run_helper.num_analyze * this->curr_time_penalty;

		/**
		 * - exit in-place to not delete existing nodes
		 */
		AbstractNode* exit_next_node;
		switch (potential_node_context->type) {
		case NODE_TYPE_ACTION:
			{
				ActionNode* action_node = (ActionNode*)potential_node_context;
				exit_next_node = action_node->next_node;
			}
			break;
		case NODE_TYPE_SCOPE:
			{
				ScopeNode* scope_node = (ScopeNode*)potential_node_context;
				exit_next_node = scope_node->next_node;
			}
			break;
		case NODE_TYPE_BRANCH:
			{
				BranchNode* branch_node = (BranchNode*)potential_node_context;
				if (potential_is_branch) {
					exit_next_node = branch_node->branch_next_node;
				} else {
					exit_next_node = branch_node->original_next_node;
				}
			}
			break;
		}

		geometric_distribution<int> geo_distribution(0.2);
		int new_num_steps = 3 + geo_distribution(generator);

		vector<int> step_types;
		vector<ActionNode*> actions;
		vector<ScopeNode*> scopes;
		/**
		 * - always give raw actions a large weight
		 *   - existing scopes often learned to avoid certain patterns
		 *     - which can prevent innovation
		 */
		uniform_int_distribution<int> scope_distribution(0, 2);
		for (int s_index = 0; s_index < new_num_steps; s_index++) {
			if (potential_node_context->parent->child_scopes.size() > 0
					&& scope_distribution(generator) == 0) {
				step_types.push_back(STEP_TYPE_SCOPE);
				actions.push_back(NULL);

				ScopeNode* new_scope_node = new ScopeNode();
				uniform_int_distribution<int> child_scope_distribution(0, potential_node_context->parent->child_scopes.size()-1);
				new_scope_node->scope = potential_node_context->parent->child_scopes[child_scope_distribution(generator)];
				scopes.push_back(new_scope_node);
			} else {
				step_types.push_back(STEP_TYPE_ACTION);

				ActionNode* new_action_node = new ActionNode();
				new_action_node->action = problem_type->random_action();
				actions.push_back(new_action_node);

				scopes.push_back(NULL);
			}
		}

		PotentialCommit* potential_commit = new PotentialCommit();
		potential_commit->node_context = potential_node_context;
		potential_commit->is_branch = potential_is_branch;
		potential_commit->step_types = step_types;
		potential_commit->actions = actions;
		potential_commit->scopes = scopes;
		potential_commit->exit_next_node = exit_next_node;

		Problem* copy_problem = problem->copy_and_reset();

		run_helper.num_analyze = 0;
		run_helper.num_actions = 0;

		#if defined(MDEBUG) && MDEBUG
		run_helper.starting_run_seed = run_index;
		run_helper.curr_run_seed = xorshift(run_helper.starting_run_seed);
		#endif /* MDEBUG */

		vector<ContextLayer> commit_context;
		this->scopes[0]->commit_activate(
			copy_problem,
			commit_context,
			run_helper,
			potential_commit);

		double new_target_val = copy_problem->score_result();
		new_target_val -= 0.05 * run_helper.num_actions * this->curr_time_penalty;
		new_target_val -= run_helper.num_analyze * this->curr_time_penalty;

		delete copy_problem;
		delete problem;

		if (new_target_val > existing_target_val) {
			potential_commit->finalize();

			delete potential_commit;

			break;
		} else {
			delete potential_commit;
		}
	}
}

void Solution::save(string path,
					string name) {
	ofstream output_file;
	output_file.open(path + "temp_" + name);

	output_file << this->timestamp << endl;
	output_file << this->curr_score << endl;

	output_file << this->curr_true_score << endl;
	output_file << this->best_true_score << endl;
	output_file << this->best_true_score_timestamp << endl;
	output_file << this->curr_time_penalty << endl;

	output_file << this->scopes.size() << endl;

	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save(output_file);
	}

	output_file << this->num_existing_scopes << endl;

	output_file << this->was_commit << endl;

	output_file.close();

	string oldname = path + "temp_" + name;
	string newname = path + name;
	rename(oldname.c_str(), newname.c_str());
}

void Solution::save_for_display(ofstream& output_file) {
	output_file << this->scopes.size() << endl;
	for (int s_index = 0; s_index < (int)this->scopes.size(); s_index++) {
		this->scopes[s_index]->save_for_display(output_file);
	}
}
