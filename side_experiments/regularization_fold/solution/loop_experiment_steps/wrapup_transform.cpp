#include "loop_experiment.h"

#include "action_node.h"
#include "exit_node.h"
#include "globals.h"
#include "scale.h"
#include "scope.h"
#include "scope_node.h"
#include "score_network.h"
#include "sequence.h"

using namespace std;

void LoopExperiment::wrapup_transform() {
	Scope* new_scope = new Scope((int)solution->scopes.size(),
								 this->new_num_states,
								 this->new_state_initialized_locally,
								 this->new_state_family_ids,
								 this->new_state_default_class_ids,
								 true,
								 this->continue_score_network,
								 this->continue_misguess_network,
								 this->halt_score_network,
								 this->halt_misguess_network);
	solution->scopes.push_back(new_scope);

	vector<Scope*> new_sequence_scopes;
	for (int l_index = 0; l_index < (int)this->sequence->scopes.size(); l_index++) {
		// can't be loop
		Scope* new_sequence_scope = new Scope((int)solution->scopes.size(),
											  this->sequence->scopes[l_index]->num_states,
											  this->sequence->scopes[l_index]->state_initialized_locally,
											  this->sequence->scopes[l_index]->state_family_ids,
											  this->sequence->scopes[l_index]->state_default_class_ids,
											  false,
											  NULL,
											  NULL,
											  NULL,
											  NULL);
		solution->scopes.push_back(new_sequence_scope);
		new_sequence_scopes.push_back(new_sequence_scope);
	}

	vector<int> new_starting_node_ids = this->sequence->starting_node_ids;
	new_starting_node_ids[0] = 0;

	vector<int> new_input_indexes;
	for (int i_index = 0; i_index < (int)this->sequence->input_types.size(); i_index++) {
		new_input_indexes.push_back(i_index);
	}

	ScopeNode* new_outer_node = new ScopeNode(new_scope,
											  0,
											  new_sequence_scopes[0]->id,
											  new_starting_node_ids,
											  new_input_indexes,
											  this->sequence->input_target_layers,
											  this->sequence->input_target_indexes,
											  this->sequence->input_has_transform,
											  this->sequence->input_transformations,
											  this->scale_mod,
											  -1);
	new_scope->nodes.push_back(new_outer_node);

	for (int l_index = 0; l_index < (int)this->sequence->scopes.size(); l_index++) {
		for (int n_index = 0; n_index < (int)this->sequence->node_ids[l_index].size()-1; n_index++) {
			AbstractNode* original_node = this->sequence->scopes[l_index]->nodes[
				this->sequence->node_ids[l_index][n_index]];
			if (original_node->type == NODE_TYPE_ACTION) {
				ActionNode* new_inner_node = new ActionNode((ActionNode*)original_node,
															new_sequence_scopes[l_index],
															n_index,
															n_index+1);
				new_sequence_scopes[l_index]->nodes.push_back(new_inner_node);
			} else if (original_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* new_inner_node = new ScopeNode((ScopeNode*)original_node,
														  new_sequence_scopes[l_index],
														  n_index,
														  n_index+1);
				new_sequence_scopes[l_index]->nodes.push_back(new_inner_node);
			} else if (original_node->type == NODE_TYPE_EXIT) {
				ExitNode* new_inner_node = new ExitNode((ExitNode*)original_node,
														new_sequence_scopes[l_index],
														n_index,
														n_index+1);
				new_sequence_scopes[l_index]->nodes.push_back(new_inner_node);
			}
		}

		if (l_index == (int)this->sequence->scopes.size()-1) {
			AbstractNode* original_node = this->sequence->scopes.back()->nodes[
				this->sequence->node_ids.back().back()];
			if (original_node->type == NODE_TYPE_ACTION) {
				ActionNode* new_inner_node = new ActionNode((ActionNode*)original_node,
															new_sequence_scopes.back(),
															(int)this->sequence->node_ids.back().size()-1,
															-1);
				new_sequence_scopes.back()->nodes.push_back(new_inner_node);
			} else if (original_node->type == NODE_TYPE_SCOPE) {
				ScopeNode* new_inner_node = new ScopeNode((ScopeNode*)original_node,
														  new_sequence_scopes.back(),
														  (int)this->sequence->node_ids.back().size()-1,
														  -1);
				new_sequence_scopes.back()->nodes.push_back(new_inner_node);
			}
			// can't be NODE_TYPE_EXIT
		} else {
			ScopeNode* original_node = (ScopeNode*)this->sequence->scopes[l_index]->nodes[
				this->sequence->node_ids[l_index].back()];
			ScopeNode* new_ending_node = new ScopeNode(original_node,
													   new_sequence_scopes[l_index],
													   (int)this->sequence->node_ids[l_index].size()-1,
													   -1);
			new_ending_node->inner_scope_id = new_sequence_scopes[l_index+1]->id;
			new_sequence_scopes[l_index]->nodes.push_back(new_ending_node);
		}
	}

	Scope* outer_scope = solution->scopes[this->scope_context.back()];
	ActionNode* original_action_node = (ActionNode*)outer_scope->nodes[this->node_context.back()];

	vector<int> new_exit_node_target_indexes;
	vector<ExitNetwork*> new_exit_node_networks;
	for (int e_index = 0; e_index < (int)this->exit_networks.size(); e_index++) {
		if (this->exit_networks[e_index] != NULL) {
			new_exit_node_target_indexes.push_back(e_index);
			new_exit_node_networks.push_back(this->exit_networks[e_index]);
		}
	}
	ExitNode* new_exit_node = new ExitNode(outer_scope,
										   (int)outer_scope->nodes.size(),
										   0,
										   original_action_node->next_node_id,
										   new_exit_node_target_indexes,
										   new_exit_node_networks);
	outer_scope->nodes.push_back(new_exit_node);

	ScopeNode* new_scope_node = new ScopeNode(outer_scope,
											  (int)outer_scope->nodes.size(),
											  new_scope->id,
											  vector<int>{0},
											  this->last_layer_indexes,
											  vector<int>(this->last_layer_indexes.size(), 0),
											  this->last_layer_target_indexes,
											  vector<bool>(this->last_layer_indexes.size(), false),
											  vector<Transformation>(this->last_layer_indexes.size(), Transformation()),
											  new Scale(),
											  new_exit_node->id);
	outer_scope->nodes.push_back(new_scope_node);

	original_action_node->next_node_id = new_scope_node->id;

	for (map<int, vector<ScoreNetwork*>>::iterator it = this->score_networks.begin();
			it != this->score_networks.end(); it++) {
		for (int n_index = 0; n_index < (int)it->second.size(); n_index++) {
			if (it->second[n_index] != NULL) {
				delete it->second[n_index];
			}
		}
	}
	delete this->sequence;

	this->state = EXPERIMENT_STATE_DONE;
}
