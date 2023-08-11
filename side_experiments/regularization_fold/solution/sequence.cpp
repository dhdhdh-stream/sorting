#include "sequence.h"

#include <iostream>

#include "abstract_experiment.h"
#include "action_node.h"
#include "constants.h"
#include "ending_scope_node_helper.h"
#include "exit_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"

using namespace std;

Sequence::Sequence(vector<Scope*> scopes,
				   vector<int> starting_node_ids,
				   vector<int> input_types,
				   vector<int> input_target_layers,
				   vector<int> input_target_indexes,
				   vector<int> input_local_scope_depths,
				   vector<int> input_local_input_indexes,
				   vector<int> input_previous_step_index,
				   vector<int> input_previous_input_index,
				   vector<int> input_last_seen_class_ids,
				   vector<bool> input_has_transform,
				   vector<Transformation> input_transformations,
				   vector<vector<int>> node_ids) {
	this->scopes = scopes;
	this->starting_node_ids = starting_node_ids;
	this->input_types = input_types;
	this->input_target_layers = input_target_layers;
	this->input_target_indexes = input_target_indexes;
	this->input_local_scope_depths = input_local_scope_depths;
	this->input_local_input_indexes = input_local_input_indexes;
	this->input_previous_step_index = input_previous_step_index;
	this->input_previous_input_index = input_previous_input_index;
	this->input_last_seen_class_ids = input_last_seen_class_ids;
	this->input_has_transform = input_has_transform;
	this->input_transformations = input_transformations;
	this->node_ids = node_ids;
}

Sequence::~Sequence() {
	// handle in experiment transforms
}

void Sequence::activate_pull(vector<double>& input_vals,
							 vector<ForwardContextLayer>& context,
							 vector<vector<double>>& previous_vals,
							 BranchExperimentHistory* branch_experiment_history,
							 RunHelper& run_helper) {
	if (this->experiment->state == EXPERIMENT_STATE_EXPLORE) {
		explore_activate_pull(input_vals,
							  context,
							  previous_vals,
							  run_helper);
	} else if (this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_activate_pull(input_vals,
								 context,
								 previous_vals,
								 branch_experiment_history,
								 run_helper);
	} else if (this->experiment->state == EXPERIMENT_STATE_MEASURE) {
		measure_activate_pull(input_vals,
							  context);
	} else if (this->experiment->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		first_clean_activate_pull(input_vals,
								  context,
								  previous_vals,
								  branch_experiment_history,
								  run_helper);
	} else if (this->experiment->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		second_clean_activate_pull(input_vals,
								   context,
								   previous_vals,
								   branch_experiment_history,
								   run_helper);
	} else {
		wrapup_activate_pull(input_vals,
							 context);
	}
}

void Sequence::activate(vector<double>& input_vals,
						vector<double>& flat_vals,
						RunHelper& run_helper,
						SequenceHistory* history) {
	vector<vector<double>> inner_state_vals(this->starting_node_ids.size());
	vector<vector<bool>> inner_states_initialized(this->starting_node_ids.size());
	inner_state_vals[0] = vector<double>(this->scopes[0]->num_states, 0.0);
	inner_states_initialized[0] = vector<bool>(this->scopes[0]->num_states, false);
	Scope* curr_scope = this->scopes[0];
	for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
		ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
		Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

		inner_state_vals[1+l_index] = vector<double>(next_scope->num_states, 0.0);
		inner_states_initialized[1+l_index] = vector<bool>(next_scope->num_states, false);

		curr_scope = next_scope;
	}
	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		double val = input_vals[i_index];
		if (this->input_has_transform[i_index]) {
			val = this->input_transformations[i_index].forward(val);
		}
		inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = val;

		inner_states_initialized[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = true;
	}

	vector<ForwardContextLayer> temp_context;
	temp_context.push_back(ForwardContextLayer());

	temp_context.back().scope_id = -1;
	temp_context.back().node_id = -1;

	temp_context.back().state_vals = &(inner_state_vals[0]);
	temp_context.back().states_initialized = inner_states_initialized[0];
	inner_states_initialized.erase(inner_states_initialized.begin());

	history->node_histories = vector<vector<AbstractNodeHistory*>>(this->scopes.size());

	if (this->starting_node_ids.size() > 1) {
		vector<int> starting_node_ids_copy = this->starting_node_ids;
		starting_node_ids_copy.erase(starting_node_ids_copy.begin());

		vector<vector<double>*> inner_state_vals_copy(this->starting_node_ids.size()-1);
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			inner_state_vals_copy[l_index] = &(inner_state_vals[1+l_index]);
		}

		// before being passed in, starting_node_ids_copy.size() == inner_state_vals_copy.size()

		ScopeNode* scope_node = (ScopeNode*)this->scopes[0]->nodes[this->node_ids[0][0]];

		// unused
		int inner_exit_depth;
		int inner_exit_node_id;

		ScopeNodeHistory* node_history = new ScopeNodeHistory(scope_node);
		history->node_histories[0].push_back(node_history);
		scope_node->halfway_activate(starting_node_ids_copy,
									 inner_state_vals_copy,
									 inner_states_initialized,
									 flat_vals,
									 temp_context,
									 inner_exit_depth,
									 inner_exit_node_id,
									 run_helper,
									 node_history);
	}

	vector<int> next_starting_node_ids;
	vector<vector<double>*> next_starting_state_vals;
	vector<vector<bool>> next_starting_states_initialized;
	vector<EndingScopeNodeActivateHelper> ending_scope_node_helpers;
	for (int l_index = 0; l_index < (int)this->scopes.size(); l_index++) {
		int scope_id = this->scopes[l_index]->id;
		vector<vector<StateNetwork*>>* scope_state_networks = &(this->experiment->state_networks.find(scope_id)->second);
		vector<ScoreNetwork*>* scope_score_networks = &(this->experiment->score_networks.find(scope_id)->second);
		// initialize on experiment start
		int scope_distance = (int)this->experiment->scope_context.size()+1;

		for (int n_index = 0; n_index < (int)this->node_ids[l_index].size(); n_index++) {
			if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 1) {
				continue;
			}

			if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
				ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
				ending_scope_node_helpers.push_back(EndingScopeNodeActivateHelper(scope_node));
				ending_scope_node_helpers.back().forward(next_starting_node_ids,
														 next_starting_state_vals,
														 next_starting_states_initialized,
														 temp_context,
														 run_helper);
			} else {
				if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
					ActionNode* action_node = (ActionNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
					ActionNodeHistory* action_node_history = new ActionNodeHistory(action_node);
					history->node_histories[l_index].push_back(action_node_history);
					action_node->activate(flat_vals,
										  temp_context,
										  scope_state_networks,
										  scope_score_networks,
										  scope_distance,
										  run_helper,
										  action_node_history);
				} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_SCOPE) {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

					// unused
					int inner_exit_depth;
					int inner_exit_node_id;

					ScopeNodeHistory* scope_node_history = new ScopeNodeHistory(scope_node);
					history->node_histories[l_index].push_back(scope_node_history);
					if (next_starting_node_ids.size() > 0) {
						// note: first node in layer
						scope_node->halfway_activate(next_starting_node_ids,
													 next_starting_state_vals,
													 next_starting_states_initialized,
													 flat_vals,
													 temp_context,
													 inner_exit_depth,
													 inner_exit_node_id,
													 run_helper,
													 scope_node_history);

						next_starting_node_ids.clear();
						next_starting_state_vals.clear();
						next_starting_states_initialized.clear();
					} else {
						scope_node->activate(flat_vals,
											 temp_context,
											 inner_exit_depth,
											 inner_exit_node_id,
											 run_helper,
											 scope_node_history);
					}
				} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_EXIT) {
					ExitNode* exit_node = (ExitNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];
					ExitNodeHistory* exit_node_history = new ExitNodeHistory(exit_node);
					history->node_histories[l_index].push_back(exit_node_history);
					exit_node->activate(temp_context,
										run_helper,
										exit_node_history);
				}
			}
		}
	}

	for (int e_index = (int)ending_scope_node_helpers.size()-1; e_index >= 0; e_index--) {
		ending_scope_node_helpers[e_index].backward(temp_context,
													run_helper);
	}

	for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
		double val = inner_state_vals[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
		if (this->input_has_transform[i_index]) {
			val = this->input_transformations[i_index].backward(val);
		}
		input_vals[i_index] = val;
	}
}

void Sequence::activate_reset(vector<double>& input_vals,
							  vector<ForwardContextLayer>& context,
							  vector<vector<double>>& previous_vals) {
	if (this->experiment->state == EXPERIMENT_STATE_EXPLORE) {
		explore_activate_reset(input_vals,
							   context,
							   previous_vals);
	} else if (this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_activate_reset(input_vals,
								  context,
								  previous_vals);
	} else if (this->experiment->state == EXPERIMENT_STATE_MEASURE) {
		measure_activate_reset(input_vals,
							   context);
	} else if (this->experiment->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		first_clean_activate_reset(input_vals,
								   context,
								   previous_vals);
	} else if (this->experiment->state == EXPERIMENT_STATE_SECOND_CLEAN) {
		second_clean_activate_reset(input_vals,
									context,
									previous_vals);
	} else {
		wrapup_activate_reset(input_vals,
							  context);
	}
}

void Sequence::backprop_pull(vector<double>& input_errors,
							 vector<BackwardContextLayer>& context,
							 vector<vector<double>>& previous_errors) {
	if (this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_backprop_pull(input_errors,
								 context,
								 previous_errors);
	} else if (this->experiment->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		first_clean_backprop_pull(input_errors,
								  context,
								  previous_errors);
	} else {
		second_clean_backprop_pull(input_errors,
								   context,
								   previous_errors);
	}
}

void Sequence::backprop(vector<double>& input_errors,
						double& scale_factor_error,
						RunHelper& run_helper,
						SequenceHistory* history) {
	if (this->experiment->state == EXPERIMENT_STATE_WRAPUP) {
		for (int l_index = 0; l_index < (int)this->scopes.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];
			run_helper.scale_factor *= scope_node->scope_scale_mod->weight;
		}

		/**
		 * - can track separate inner_scale_factor_errors and update separate scope nodes/scale mods
		 *   - but will simply track cumulative for now, and let separate adjust after experiment
		 */
		double cumulative_scale_factor_error = 0.0;
		for (int l_index = (int)this->scopes.size()-1; l_index >= 0; l_index--) {
			for (int n_index = (int)this->node_ids[l_index].size()-1; n_index >= 0; n_index--) {
				if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
					ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]];

					cumulative_scale_factor_error *= scope_node->scope_scale_mod->weight;

					run_helper.scale_factor /= scope_node->scope_scale_mod->weight;
				} else {
					if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[l_index][n_index];
						ActionNode* action_node = (ActionNode*)action_node_history->node;
						vector<BackwardContextLayer> empty_context;
						action_node->backprop(empty_context,
											  cumulative_scale_factor_error,
											  run_helper,
											  action_node_history);
					} else {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[l_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
						vector<BackwardContextLayer> empty_context;
						scope_node->backprop(empty_context,
											 cumulative_scale_factor_error,
											 run_helper,
											 scope_node_history);
					}
				}
			}
		}

		scale_factor_error = cumulative_scale_factor_error;
	} else {
		vector<vector<double>> inner_state_errors(this->starting_node_ids.size());
		inner_state_errors[0] = vector<double>(this->scopes[0]->num_states, 0.0);
		Scope* curr_scope = this->scopes[0];
		for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)curr_scope->nodes[this->starting_node_ids[l_index]];
			Scope* next_scope = solution->scopes[scope_node->inner_scope_id];

			inner_state_errors[1+l_index] = vector<double>(next_scope->num_states, 0.0);

			curr_scope = next_scope;
		}
		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			double error = input_errors[i_index];
			if (this->input_has_transform[i_index]) {
				error = this->input_transformations[i_index].backprop_backward(error);
			}
			inner_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]] = error;
		}

		vector<BackwardContextLayer> temp_context;
		temp_context.push_back(BackwardContextLayer());

		temp_context.back().state_errors = &(inner_state_errors[0]);

		vector<int> next_starting_node_ids;
		vector<vector<double>*> next_starting_state_errors;
		vector<EndingScopeNodeBackpropHelper> ending_scope_node_helpers;
		for (int l_index = 0; l_index < (int)this->scopes.size()-1; l_index++) {
			ScopeNode* scope_node = (ScopeNode*)this->scopes[l_index]->nodes[this->node_ids[l_index].back()];
			ending_scope_node_helpers.push_back(EndingScopeNodeBackpropHelper(scope_node));
			ending_scope_node_helpers.back().backward(next_starting_node_ids,
													  next_starting_state_errors,
													  temp_context,
													  run_helper);
		}

		double cumulative_scale_factor_error = 0.0;
		for (int l_index = (int)this->scopes.size()-1; l_index >= 0; l_index--) {
			for (int n_index = (int)this->node_ids[l_index].size()-1; n_index >= 0; n_index--) {
				if (l_index == 0 && n_index == 0 && this->starting_node_ids.size() > 1) {
					continue;	// i.e., break
				}

				if (n_index == (int)this->node_ids[l_index].size()-1 && l_index != (int)this->scopes.size()-1) {
					ending_scope_node_helpers[l_index].forward(temp_context,
															   cumulative_scale_factor_error,
															   run_helper);
				} else {
					if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_ACTION) {
						ActionNodeHistory* action_node_history = (ActionNodeHistory*)history->node_histories[l_index][n_index];
						ActionNode* action_node = (ActionNode*)action_node_history->node;
						action_node->backprop(temp_context,
											  cumulative_scale_factor_error,
											  run_helper,
											  action_node_history);
					} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_SCOPE) {
						ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[l_index][n_index];
						ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
						if (n_index == 0 && next_starting_node_ids.size() > 0) {
							scope_node->halfway_backprop(next_starting_node_ids,
														 next_starting_state_errors,
														 temp_context,
														 cumulative_scale_factor_error,
														 run_helper,
														 scope_node_history);

							next_starting_node_ids.clear();
							next_starting_state_errors.clear();
						} else {
							scope_node->backprop(temp_context,
												 cumulative_scale_factor_error,
												 run_helper,
												 scope_node_history);
						}
					} else if (this->scopes[l_index]->nodes[this->node_ids[l_index][n_index]]->type == NODE_TYPE_EXIT) {
						ExitNodeHistory* exit_node_history = (ExitNodeHistory*)history->node_histories[l_index][n_index];
						ExitNode* exit_node = (ExitNode*)exit_node_history->node;
						exit_node->backprop(temp_context,
											run_helper,
											exit_node_history);
					}
				}
			}
		}

		if (this->starting_node_ids.size() > 1) {
			vector<int> starting_node_ids_copy = this->starting_node_ids;
			starting_node_ids_copy.erase(starting_node_ids_copy.begin());

			vector<vector<double>*> inner_state_errors_copy(this->starting_node_ids.size()-1);
			for (int l_index = 0; l_index < (int)this->starting_node_ids.size()-1; l_index++) {
				inner_state_errors_copy[l_index] = &(inner_state_errors[1+l_index]);
			}

			ScopeNodeHistory* scope_node_history = (ScopeNodeHistory*)history->node_histories[0][0];
			ScopeNode* scope_node = (ScopeNode*)scope_node_history->node;
			scope_node->halfway_backprop(starting_node_ids_copy,
										 inner_state_errors_copy,
										 temp_context,
										 cumulative_scale_factor_error,
										 run_helper,
										 scope_node_history);
		}

		scale_factor_error = cumulative_scale_factor_error;

		for (int i_index = 0; i_index < (int)this->input_types.size(); i_index++) {
			double error = inner_state_errors[this->input_target_layers[i_index]][this->input_target_indexes[i_index]];
			if (this->input_has_transform[i_index]) {
				error = this->input_transformations[i_index].backprop_forward(error);
			}
			input_errors[i_index] = error;
		}
	}
}

void Sequence::backprop_reset(vector<double>& input_errors,
							  vector<BackwardContextLayer>& context,
							  vector<vector<double>>& previous_errors) {
	if (this->experiment->state == EXPERIMENT_STATE_EXPERIMENT) {
		experiment_backprop_reset(input_errors,
								  context,
								  previous_errors);
	} else if (this->experiment->state == EXPERIMENT_STATE_FIRST_CLEAN) {
		first_clean_backprop_reset(input_errors,
								   context,
								   previous_errors);
	} else {
		second_clean_backprop_reset(input_errors,
									context,
									previous_errors);
	}
}

SequenceHistory::SequenceHistory(Sequence* sequence) {
	this->sequence = sequence;
}

SequenceHistory::~SequenceHistory() {
	for (int l_index = 0; l_index < (int)this->node_histories.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->node_histories[l_index].size(); n_index++) {
			delete this->node_histories[l_index][n_index];
		}
	}
}
