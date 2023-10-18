#include "branch_experiment.h"

#include <iostream>

#include "action_node.h"
#include "branch_node.h"
#include "constants.h"
#include "exit_node.h"
#include "globals.h"
#include "helpers.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "sequence.h"
#include "solution.h"
#include "state.h"
#include "state_network.h"

using namespace std;

const int MEASURE_ITERS = 10000;

void BranchExperiment::measure_existing_activate(vector<ContextLayer>& context,
												 BranchExperimentHistory* history) {
	double original_score = this->existing_average_score;
	double branch_score = this->new_average_score;

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		double normalized = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;

		map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.find(it->first);
		if (existing_it != this->existing_starting_score_state_scales.end()) {
			original_score += existing_it->second->weight * normalized;
		}
		map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(it->first);
		if (new_it != this->new_starting_score_state_scales.end()) {
			branch_score += new_it->second->weight * normalized;
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_score_state_vals.end(); it++) {
		map<State*, Scale*>::iterator scale_it = this->new_starting_experiment_score_state_scales.find(it->first);
		if (scale_it != this->new_starting_experiment_score_state_scales.end()) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			branch_score += scale_it->second->weight * normalized;
		}
	}

	if (branch_score > original_score) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}
}

void BranchExperiment::measure_existing_backprop(double target_val,
												 BranchExperimentHistory* history) {
	if (history->is_branch) {
		this->branch_existing_score += target_val;

		this->existing_branch_count++;
	} else {
		this->non_branch_existing_score += target_val;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		this->state = BRANCH_EXPERIMENT_STATE_MEASURE_NEW;
		this->state_iter = 0;
	}
}

void BranchExperiment::measure_new_activate(int& curr_node_id,
											Problem& problem,
											vector<ContextLayer>& context,
											int& exit_depth,
											int& exit_node_id,
											RunHelper& run_helper,
											BranchExperimentHistory* history) {
	double original_score = this->existing_average_score;
	double branch_score = this->new_average_score;

	for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
			it != context.back().input_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.find(it->first);
			if (existing_it != this->existing_starting_input_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(it->first);
			if (new_it != this->new_starting_input_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
			it != context.back().local_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		if (last_network != NULL) {
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;

			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * normalized;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * normalized;
			}
		} else {
			map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.find(it->first);
			if (existing_it != this->existing_starting_local_state_scales.end()) {
				original_score += existing_it->second->weight * it->second.val;
			}
			map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(it->first);
			if (new_it != this->new_starting_local_state_scales.end()) {
				branch_score += new_it->second->weight * it->second.val;
			}
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].score_state_vals.end(); it++) {
		StateNetwork* last_network = it->second.last_network;
		// last_network != NULL
		double normalized = (it->second.val - last_network->ending_mean)
			/ last_network->ending_standard_deviation;

		map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.find(it->first);
		if (existing_it != this->existing_starting_score_state_scales.end()) {
			original_score += existing_it->second->weight * normalized;
		}
		map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(it->first);
		if (new_it != this->new_starting_score_state_scales.end()) {
			branch_score += new_it->second->weight * normalized;
		}
	}

	for (map<State*, StateStatus>::iterator it = context[context.size() - this->scope_context.size()].experiment_score_state_vals.begin();
			it != context[context.size() - this->scope_context.size()].experiment_score_state_vals.end(); it++) {
		map<State*, Scale*>::iterator scale_it = this->new_starting_experiment_score_state_scales.find(it->first);
		if (scale_it != this->new_starting_experiment_score_state_scales.end()) {
			StateNetwork* last_network = it->second.last_network;
			// last_network != NULL
			double normalized = (it->second.val - last_network->ending_mean)
				/ last_network->ending_standard_deviation;
			branch_score += scale_it->second->weight * normalized;
		}
	}

	if (branch_score > original_score) {
		history->is_branch = true;
	} else {
		history->is_branch = false;
	}

	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->branch_experiment_simple_activate(
				problem);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::measure_new_backprop(double target_val,
											BranchExperimentHistory* history) {
	if (history->is_branch) {
		this->branch_new_score += target_val;

		this->new_branch_count++;
	} else {
		this->non_branch_new_score += target_val;
	}

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		this->state = BRANCH_EXPERIMENT_STATE_MEASURE_PASS_THROUGH;
		this->state_iter = 0;
	}
}

void BranchExperiment::measure_pass_through_activate(
		int& curr_node_id,
		Problem& problem,
		vector<ContextLayer>& context,
		int& exit_depth,
		int& exit_node_id,
		RunHelper& run_helper) {
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		// leave context.back().node_id as -1

		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->branch_experiment_simple_activate(
				problem);
		} else {
			SequenceHistory* sequence_history = new SequenceHistory(this->best_sequences[s_index]);
			this->best_sequences[s_index]->activate(problem,
													context,
													run_helper,
													sequence_history);
			delete sequence_history;
		}
	}

	if (this->best_exit_depth == 0) {
		curr_node_id = this->best_exit_node_id;
	} else {
		exit_depth = this->best_exit_depth-1;
		exit_node_id = this->best_exit_node_id;
	}
}

void BranchExperiment::measure_pass_through_backprop(double target_val) {
	this->pass_through_score += target_val;

	this->state_iter++;
	if (this->state_iter >= MEASURE_ITERS) {
		eval();
	}
}

void BranchExperiment::eval() {
	cout << "new explore path:";
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			cout << " " << this->best_actions[s_index]->action.to_string();
		} else {
			cout << " S";
		}
	}
	cout << endl;

	cout << "this->best_exit_depth: " << this->best_exit_depth << endl;
	cout << "this->best_exit_node_id: " << this->best_exit_node_id << endl;

	Scope* parent = solution->scopes[this->scope_context[0]];

	double score_standard_deviation = sqrt(parent->score_variance);

	double branch_existing_average_score = this->branch_existing_score / this->existing_branch_count;
	double branch_new_average_score = this->branch_new_score / this->new_branch_count;
	double branch_improvement = branch_new_average_score - branch_existing_average_score;
	double branch_count = (this->existing_branch_count + this->new_branch_count) / 2.0;
	double branch_improvement_t_score = branch_improvement
		/ (score_standard_deviation / sqrt(branch_count));

	cout << "score_standard_deviation: " << score_standard_deviation << endl;
	cout << "branch_existing_average_score: " << branch_existing_average_score << endl;
	cout << "branch_new_average_score: " << branch_new_average_score << endl;
	cout << "branch_improvement_t_score: " << branch_improvement_t_score << endl;
	cout << "this->existing_branch_count: " << this->existing_branch_count << endl;
	cout << "this->new_branch_count: " << this->new_branch_count << endl;

	double non_branch_existing_average_score = this->non_branch_existing_score / (MEASURE_ITERS - this->existing_branch_count);
	double non_branch_new_average_score = this->non_branch_new_score / (MEASURE_ITERS - this->new_branch_count);
	double non_branch_improvement = non_branch_new_average_score - non_branch_existing_average_score;
	double non_branch_count = MEASURE_ITERS - branch_count;
	double non_branch_improvement_t_score = non_branch_improvement
		/ (score_standard_deviation / sqrt(non_branch_count));

	cout << "non_branch_existing_average_score: " << non_branch_existing_average_score << endl;
	cout << "non_branch_new_average_score: " << non_branch_new_average_score << endl;
	cout << "non_branch_improvement_t_score: " << non_branch_improvement_t_score << endl;

	double branch_weight = branch_count / MEASURE_ITERS;

	if (branch_weight > 0.02
			&& branch_improvement_t_score > 2.326) {	// >99%
		if (branch_weight > 0.98
				|| non_branch_improvement_t_score > -0.674) {	// 75%<
			new_pass_through();
		} else {
			new_branch();
		}

		ofstream solution_save_file;
		solution_save_file.open("saves/solution.txt");
		solution->save(solution_save_file);
		solution_save_file.close();

		ofstream display_file;
		display_file.open("../display.txt");
		solution->save_for_display(display_file);
		display_file.close();
	} else {
		double pass_through_average_score = this->pass_through_score / MEASURE_ITERS;

		double combined_improvement = pass_through_average_score - this->existing_average_score;
		double combined_improvement_t_score = combined_improvement
			/ (score_standard_deviation / sqrt(MEASURE_ITERS));

		cout << "pass_through_average_score: " << pass_through_average_score << endl;
		cout << "this->existing_average_score: " << this->existing_average_score << endl;
		cout << "combined_improvement_t_score: " << combined_improvement_t_score << endl;

		double misguess_improvement = this->existing_average_misguess - this->new_average_misguess;
		double misguess_standard_deviation = sqrt(parent->misguess_variance);
		// 0.0001 rolling average variance approx. equal to 20000 average variance (?)
		double misguess_improvement_t_score = misguess_improvement
			/ (misguess_standard_deviation / sqrt(20000));

		cout << "this->new_average_misguess: " << this->new_average_misguess << endl;
		cout << "misguess_improvement: " << misguess_improvement << endl;
		cout << "misguess_standard_deviation: " << misguess_standard_deviation << endl;
		cout << "misguess_improvement_t_score: " << misguess_improvement_t_score << endl;

		if (combined_improvement_t_score > -0.674
				&& misguess_improvement_t_score > 2.326) {
			new_pass_through();

			ofstream solution_save_file;
			solution_save_file.open("saves/solution.txt");
			solution->save(solution_save_file);
			solution_save_file.close();

			ofstream display_file;
			display_file.open("../display.txt");
			solution->save_for_display(display_file);
			display_file.close();
		} else {
			for (map<int, Scale*>::iterator it = this->existing_starting_input_state_scales.begin();
					it != this->existing_starting_input_state_scales.end(); it++) {
				delete it->second;
			}
			this->existing_starting_input_state_scales.clear();
			for (map<int, Scale*>::iterator it = this->existing_starting_local_state_scales.begin();
					it != this->existing_starting_local_state_scales.end(); it++) {
				delete it->second;
			}
			this->existing_starting_local_state_scales.clear();
			for (map<State*, Scale*>::iterator it = this->existing_starting_score_state_scales.begin();
					it != this->existing_starting_score_state_scales.end(); it++) {
				delete it->second;
			}
			this->existing_starting_score_state_scales.clear();

			for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
				if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
					delete this->best_actions[s_index];
				} else {
					delete this->best_sequences[s_index];
				}
			}
			this->best_actions.clear();
			this->best_sequences.clear();

			for (map<int, Scale*>::iterator it = this->new_starting_input_state_scales.begin();
					it != this->new_starting_input_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_starting_input_state_scales.clear();
			for (map<int, Scale*>::iterator it = this->new_starting_local_state_scales.begin();
					it != this->new_starting_local_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_starting_local_state_scales.clear();
			for (map<State*, Scale*>::iterator it = this->new_starting_score_state_scales.begin();
					it != this->new_starting_score_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_starting_score_state_scales.clear();
			for (map<State*, Scale*>::iterator it = this->new_starting_experiment_score_state_scales.begin();
					it != this->new_starting_experiment_score_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_starting_experiment_score_state_scales.clear();

			for (map<int, Scale*>::iterator it = this->new_ending_input_state_scales.begin();
					it != this->new_ending_input_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_ending_input_state_scales.clear();
			for (map<int, Scale*>::iterator it = this->new_ending_local_state_scales.begin();
					it != this->new_ending_local_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_ending_local_state_scales.clear();
			for (map<State*, Scale*>::iterator it = this->new_ending_score_state_scales.begin();
					it != this->new_ending_score_state_scales.end(); it++) {
				delete it->second;
			}
			this->new_ending_score_state_scales.clear();

			for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
				delete this->new_score_states[s_index];
			}
			this->new_score_states.clear();

			for (map<State*, pair<Scale*, double>>::iterator it = this->new_score_state_scales.begin();
					it != this->new_score_state_scales.end(); it++) {
				delete it->second.first;
			}
			this->new_score_state_scales.clear();
		}
	}

	cout << endl;

	this->state = BRANCH_EXPERIMENT_STATE_DONE;
}

void BranchExperiment::new_branch() {
	cout << "new_branch" << endl;

	Scope* starting_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = false;

	new_branch_node->original_score_mod = this->existing_average_score;
	new_branch_node->branch_score_mod = this->new_average_score;

	for (map<int, Scale*>::iterator existing_it = this->existing_starting_input_state_scales.begin();
			existing_it != this->existing_starting_input_state_scales.end(); existing_it++) {
		map<int, Scale*>::iterator new_it = this->new_starting_input_state_scales.find(existing_it->first);
		new_branch_node->decision_state_is_local.push_back(false);
		new_branch_node->decision_state_indexes.push_back(existing_it->first);
		new_branch_node->decision_original_weights.push_back(existing_it->second->weight);
		new_branch_node->decision_branch_weights.push_back(new_it->second->weight);
		delete existing_it->second;
		delete new_it->second;
	}
	this->existing_starting_input_state_scales.clear();
	this->new_starting_input_state_scales.clear();

	for (map<int, Scale*>::iterator existing_it = this->existing_starting_local_state_scales.begin();
			existing_it != this->existing_starting_local_state_scales.end(); existing_it++) {
		map<int, Scale*>::iterator new_it = this->new_starting_local_state_scales.find(existing_it->first);
		new_branch_node->decision_state_is_local.push_back(true);
		new_branch_node->decision_state_indexes.push_back(existing_it->first);
		new_branch_node->decision_original_weights.push_back(existing_it->second->weight);
		new_branch_node->decision_branch_weights.push_back(new_it->second->weight);
		delete existing_it->second;
		delete new_it->second;
	}
	this->existing_starting_local_state_scales.clear();
	this->new_starting_local_state_scales.clear();

	if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)starting_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)starting_scope->nodes.size();

	new_branch_node->recursion_protection = this->recursion_protection && this->need_recursion_protection;
	cout << "new_branch_node->recursion_protection: " << new_branch_node->recursion_protection << endl;

	map<int, ScopeNode*> sequence_scope_node_mappings;
	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)starting_scope->nodes.size();
		} else {
			ScopeNode* new_sequence_scope_node = finalize_sequence(
				this->scope_context,
				this->node_context,
				this->best_sequences[s_index],
				input_scope_depths_mappings,
				output_scope_depths_mappings);
			new_sequence_scope_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_sequence_scope_node);

			new_sequence_scope_node->next_node_id = (int)starting_scope->nodes.size();

			delete this->best_sequences[s_index];

			starting_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);

			sequence_scope_node_mappings[new_sequence_scope_node->inner_scope->id] = new_sequence_scope_node;
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	ExitNode* new_exit_node = new ExitNode();

	new_exit_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_exit_node);

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_id = this->best_exit_node_id;

	double score_standard_deviation = sqrt(parent_scope->score_variance);
	for (map<State*, Scale*>::iterator it = this->new_ending_score_state_scales.begin();
			it != this->new_ending_score_state_scales.end(); it++) {
		map<State*, Scale*>::iterator existing_it = this->existing_starting_score_state_scales.find(it->first);
		map<State*, Scale*>::iterator new_it = this->new_starting_score_state_scales.find(it->first);

		bool still_exists = parent_scope->score_state_scales.find(it->first) != parent_scope->score_state_scales.end();
		if (still_exists) {
			if (existing_it == this->existing_starting_score_state_scales.end()) {
				double original_impact = parent_scope->score_state_scales.find(it->first)->second.first->weight;
				double new_impact = it->second->weight;
				if (abs(original_impact - new_impact) > MIN_SCORE_IMPACT*score_standard_deviation) {
					// remove score state
					it->first->detach(parent_scope);
					delete it->first;
				} else {
					// leave score state as-is
				}
			} else {
				finalize_existing_state(parent_scope,
										it->first,
										new_branch_node,
										existing_it->second->weight,
										new_it->second->weight);
			}
		}
		/**
		 * - if score state was too low impact and removed, then simply assume not needed for decision
		 * 
		 * - if was added by another branch experiment, then measure was reset, so decisions were OK even without it
		 */

		delete it->second;
		if (existing_it != this->existing_starting_score_state_scales.end()) {
			delete existing_it->second;
		}
		if (new_it != this->new_starting_score_state_scales.end()) {
			delete new_it->second;
		}
	}
	this->existing_starting_score_state_scales.clear();
	this->new_starting_score_state_scales.clear();
	this->new_ending_score_state_scales.clear();

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		map<State*, Scale*>::iterator start_it = this->new_starting_experiment_score_state_scales.find(this->new_score_states[s_index]);
		if (start_it == this->new_starting_experiment_score_state_scales.end()) {
			// don't add
			delete this->new_score_states[s_index];
		} else {
			finalize_new_state(parent_scope,
							   sequence_scope_node_mappings,
							   this->new_score_states[s_index],
							   this->new_score_state_nodes[s_index],
							   this->new_score_state_scope_contexts[s_index],
							   this->new_score_state_node_contexts[s_index],
							   this->new_score_state_obs_indexes[s_index],
							   new_branch_node,
							   start_it->second->weight);

			delete start_it->second;
		}

		delete this->new_score_state_scales[this->new_score_states[s_index]].first;
	}
	this->new_starting_experiment_score_state_scales.clear();
	this->new_score_states.clear();
	this->new_score_state_scales.clear();
}

void BranchExperiment::new_pass_through() {
	cout << "new_pass_through" << endl;

	Scope* starting_scope = solution->scopes[this->scope_context.back()];
	Scope* parent_scope = solution->scopes[this->scope_context[0]];

	BranchNode* new_branch_node = new BranchNode();
	new_branch_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_branch_node);

	new_branch_node->branch_scope_context = this->scope_context;
	new_branch_node->branch_node_context = this->node_context;
	new_branch_node->branch_node_context.back() = new_branch_node->id;

	new_branch_node->branch_is_pass_through = true;

	new_branch_node->original_score_mod = 0.0;
	new_branch_node->branch_score_mod = 0.0;

	for (map<int, Scale*>::iterator it = this->existing_starting_input_state_scales.begin();
			it != this->existing_starting_input_state_scales.end(); it++) {
		delete it->second;
	}
	this->existing_starting_input_state_scales.clear();
	for (map<int, Scale*>::iterator it = this->new_starting_input_state_scales.begin();
			it != this->new_starting_input_state_scales.end(); it++) {
		delete it->second;
	}
	this->new_starting_input_state_scales.clear();

	for (map<int, Scale*>::iterator it = this->existing_starting_local_state_scales.begin();
			it != this->existing_starting_local_state_scales.end(); it++) {
		delete it->second;
	}
	this->existing_starting_local_state_scales.clear();
	for (map<int, Scale*>::iterator it = this->new_starting_local_state_scales.begin();
			it != this->new_starting_local_state_scales.end(); it++) {
		delete it->second;
	}
	this->new_starting_local_state_scales.clear();

	if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_ACTION) {
		ActionNode* action_node = (ActionNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = action_node->next_node_id;

		action_node->next_node_id = new_branch_node->id;
	} else if (starting_scope->nodes[this->node_context.back()]->type == NODE_TYPE_SCOPE) {
		ScopeNode* scope_node = (ScopeNode*)starting_scope->nodes[this->node_context.back()];

		new_branch_node->original_next_node_id = scope_node->next_node_id;

		scope_node->next_node_id = new_branch_node->id;
	} else {
		BranchNode* branch_node = (BranchNode*)starting_scope->nodes[this->node_context.back()];

		if (branch_node->experiment_is_branch) {
			new_branch_node->original_next_node_id = branch_node->branch_next_node_id;

			branch_node->branch_next_node_id = new_branch_node->id;
		} else {
			new_branch_node->original_next_node_id = branch_node->original_next_node_id;

			branch_node->original_next_node_id = new_branch_node->id;
		}
	}
	new_branch_node->branch_next_node_id = (int)starting_scope->nodes.size();

	new_branch_node->recursion_protection = this->recursion_protection && this->need_recursion_protection;
	cout << "new_branch_node->recursion_protection: " << new_branch_node->recursion_protection << endl;

	map<int, ScopeNode*> sequence_scope_node_mappings;
	map<pair<int, pair<bool,int>>, int> input_scope_depths_mappings;
	map<pair<int, pair<bool,int>>, int> output_scope_depths_mappings;
	for (int s_index = 0; s_index < (int)this->best_step_types.size(); s_index++) {
		if (this->best_step_types[s_index] == STEP_TYPE_ACTION) {
			this->best_actions[s_index]->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(this->best_actions[s_index]);

			this->best_actions[s_index]->next_node_id = (int)starting_scope->nodes.size();
		} else {
			ScopeNode* new_sequence_scope_node = finalize_sequence(
				this->scope_context,
				this->node_context,
				this->best_sequences[s_index],
				input_scope_depths_mappings,
				output_scope_depths_mappings);
			new_sequence_scope_node->id = (int)starting_scope->nodes.size();
			starting_scope->nodes.push_back(new_sequence_scope_node);

			new_sequence_scope_node->next_node_id = (int)starting_scope->nodes.size();

			delete this->best_sequences[s_index];

			starting_scope->child_scopes.push_back(new_sequence_scope_node->inner_scope);

			sequence_scope_node_mappings[new_sequence_scope_node->inner_scope->id] = new_sequence_scope_node;
		}
	}
	this->best_actions.clear();
	this->best_sequences.clear();

	ExitNode* new_exit_node = new ExitNode();

	new_exit_node->id = (int)starting_scope->nodes.size();
	starting_scope->nodes.push_back(new_exit_node);

	new_exit_node->exit_depth = this->best_exit_depth;
	new_exit_node->exit_node_id = this->best_exit_node_id;

	for (map<State*, Scale*>::iterator it = this->existing_starting_score_state_scales.begin();
			it != this->existing_starting_score_state_scales.end(); it++) {
		delete it->second;
	}
	this->existing_starting_score_state_scales.clear();
	for (map<State*, Scale*>::iterator it = this->new_starting_score_state_scales.begin();
			it != this->new_starting_score_state_scales.end(); it++) {
		delete it->second;
	}
	this->new_starting_score_state_scales.clear();
	for (map<State*, Scale*>::iterator it = this->new_ending_score_state_scales.begin();
			it != this->new_ending_score_state_scales.end(); it++) {
		delete it->second;
		/**
		 * - delete experiment scales as may not be general
		 *   - let parent_scope adjust after
		 */
	}
	this->new_ending_score_state_scales.clear();

	for (int s_index = 0; s_index < (int)this->new_score_states.size(); s_index++) {
		for (int n_index = 0; n_index < (int)this->new_score_state_nodes[s_index].size(); n_index++) {
			if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_ACTION) {
				ActionNode* action_node = (ActionNode*)this->new_score_state_nodes[s_index][n_index];

				action_node->score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				action_node->score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				for (int c_index = 0; c_index < (int)action_node->score_state_node_contexts.back().size()-1; c_index++) {
					if (action_node->score_state_node_contexts.back()[c_index] == -1) {
						int inner_scope_id = action_node->score_state_scope_contexts.back()[c_index+1];
						ScopeNode* new_sequence_scope_node = sequence_scope_node_mappings[inner_scope_id];
						action_node->score_state_node_contexts.back()[c_index] = new_sequence_scope_node->id;
						break;
					}
				}
				if (action_node->score_state_node_contexts.back().back() == -1) {
					action_node->score_state_node_contexts.back().back() = action_node->id;
				}
				action_node->score_state_defs.push_back(this->new_score_states[s_index]);
				action_node->score_state_network_indexes.push_back(n_index);
			} else if (this->new_score_state_nodes[s_index][n_index]->type == NODE_TYPE_SCOPE) {
				ScopeNode* scope_node = (ScopeNode*)this->new_score_state_nodes[s_index][n_index];

				scope_node->score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				scope_node->score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				for (int c_index = 0; c_index < (int)scope_node->score_state_node_contexts.back().size()-1; c_index++) {
					if (scope_node->score_state_node_contexts.back()[c_index] == -1) {
						int inner_scope_id = scope_node->score_state_scope_contexts.back()[c_index+1];
						ScopeNode* new_sequence_scope_node = sequence_scope_node_mappings[inner_scope_id];
						scope_node->score_state_node_contexts.back()[c_index] = new_sequence_scope_node->id;
						break;
					}
				}
				scope_node->score_state_obs_indexes.push_back(this->new_score_state_obs_indexes[s_index][n_index]);
				scope_node->score_state_defs.push_back(this->new_score_states[s_index]);
				scope_node->score_state_network_indexes.push_back(n_index);
			} else {
				BranchNode* branch_node = (BranchNode*)this->new_score_state_nodes[s_index][n_index];

				branch_node->score_state_scope_contexts.push_back(this->new_score_state_scope_contexts[s_index][n_index]);
				branch_node->score_state_node_contexts.push_back(this->new_score_state_node_contexts[s_index][n_index]);
				for (int c_index = 0; c_index < (int)branch_node->score_state_node_contexts.back().size()-1; c_index++) {
					if (branch_node->score_state_node_contexts.back()[c_index] == -1) {
						int inner_scope_id = branch_node->score_state_scope_contexts.back()[c_index+1];
						ScopeNode* new_sequence_scope_node = sequence_scope_node_mappings[inner_scope_id];
						branch_node->score_state_node_contexts.back()[c_index] = new_sequence_scope_node->id;
						break;
					}
				}
				branch_node->score_state_defs.push_back(this->new_score_states[s_index]);
				branch_node->score_state_network_indexes.push_back(n_index);
			}
		}

		parent_scope->score_state_scales[this->new_score_states[s_index]] = this->new_score_state_scales[this->new_score_states[s_index]];
		parent_scope->score_state_nodes[this->new_score_states[s_index]] = this->new_score_state_nodes[s_index];

		solution->states[this->new_score_states[s_index]->id] = this->new_score_states[s_index];
	}
	this->new_score_states.clear();
	this->new_score_state_scales.clear();
}
