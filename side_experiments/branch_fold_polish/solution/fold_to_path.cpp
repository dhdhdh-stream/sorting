#include "fold_to_path.h"

#include <iostream>

#include "definitions.h"

using namespace std;

Scope* construct_scope_helper(vector<FinishedStep*> finished_steps,
							  int curr_layer,
							  vector<int>& outer_input_layer,
							  vector<int>& outer_input_sizes,
							  vector<FoldNetwork*>& outer_input_networks,
							  FoldNetwork*& outer_score_network,
							  double& outer_average_score,
							  double& outer_average_misguess,
							  double& outer_average_local_impact,
							  bool& outer_active_compress,
							  int& outer_compress_new_size,
							  FoldNetwork*& outer_compress_network,
							  int& outer_compress_original_size,
							  vector<int>& compressed_scope_sizes) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	int curr_start = -1;
	int num_scopes = 0;
	for (int n_index = 0; n_index < (int)finished_steps.size(); n_index++) {
		num_scopes++;
		if (finished_steps[n_index]->compress_num_layers > 0) {
			num_scopes -= finished_steps[n_index]->compress_num_layers;
			if (finished_steps[n_index]->compress_new_size > 0) {
				num_scopes++;
			}
		}

		if (num_scopes > 1) {
			if (curr_start == -1) {
				curr_start = n_index;
			}
		} else {
			// num_scopes can be < 0 if end of scope
			if (curr_start != -1) {
				scope_starts.push_back(curr_start);
				scope_ends.push_back(n_index);

				curr_start = -1;
			}
		}
	}

	cout << "scopes:" << endl;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		cout << "start: " << scope_starts[s_index] << endl;
		cout << "end: " << scope_ends[s_index] << endl;
	}
	cout << endl;

	int scope_num_inputs = 0;
	int scope_num_outputs;

	int scope_sequence_length = 0;
	vector<bool> scope_is_inner_scope;
	vector<Scope*> scope_scopes;
	vector<int> scope_obs_sizes;

	vector<vector<FoldNetwork*>> scope_inner_input_networks;
	vector<vector<int>> scope_inner_input_sizes;
	vector<double> scope_scope_scale_mod;

	vector<int> scope_step_types;	// initially STEP_TYPE_STEP
	vector<Branch*> scope_branches;	// initially NULL
	vector<Fold*> scope_folds;		// initially NULL

	vector<FoldNetwork*> scope_score_networks;

	vector<double> scope_average_scores;
	vector<double> scope_average_misguesses;
	vector<double> scope_average_inner_scope_impacts;
	vector<double> scope_average_local_impacts;
	vector<double> scope_average_inner_branch_impacts;	// initially doesn't matter

	vector<bool> scope_active_compress;
	vector<int> scope_compress_new_sizes;
	vector<FoldNetwork*> scope_compress_networks;
	vector<int> scope_compress_original_sizes;

	int n_index = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				scope_sequence_length++;
				scope_is_inner_scope.push_back(finished_steps[n_index]->is_inner_scope);
				scope_scopes.push_back(finished_steps[n_index]->scope);
				finished_steps[n_index]->scope = NULL;	// for garbage collection
				scope_obs_sizes.push_back(finished_steps[n_index]->obs_size);

				scope_inner_input_networks.push_back(vector<FoldNetwork*>());
				scope_inner_input_sizes.push_back(vector<int>());
				scope_scope_scale_mod.push_back(finished_steps[n_index]->scope_scale_mod);
				if (finished_steps[n_index]->is_inner_scope) {
					scope_inner_input_networks.back().push_back(finished_steps[n_index]->inner_input_network);
					finished_steps[n_index]->inner_input_network = NULL;	// for garbage collection
					scope_inner_input_sizes.back().push_back(finished_steps[n_index]->scope->num_inputs);

					for (int i_index = 0; i_index < (int)finished_steps[n_index]->inner_input_input_networks.size(); i_index++) {
						if (finished_steps[n_index]->inner_input_input_layer[i_index] == curr_layer-1) {
							scope_num_inputs += finished_steps[n_index]->inner_input_input_sizes[i_index];
						}

						outer_input_layer.push_back(finished_steps[n_index]->inner_input_input_layer[i_index]);
						outer_input_sizes.push_back(finished_steps[n_index]->inner_input_input_sizes[i_index]);
						outer_input_networks.push_back(finished_steps[n_index]->inner_input_input_networks[i_index]);
					}
				}
				finished_steps[n_index]->inner_input_input_networks.clear();	// for garbage collection

				scope_step_types.push_back(STEP_TYPE_STEP);
				scope_branches.push_back(NULL);
				scope_folds.push_back(NULL);

				for (int i_index = 0; i_index < (int)finished_steps[n_index]->input_networks.size(); i_index++) {
					if (finished_steps[n_index]->input_layer[i_index] == curr_layer-1) {
						scope_num_inputs += finished_steps[n_index]->input_sizes[i_index];
					}

					outer_input_layer.push_back(finished_steps[n_index]->input_layer[i_index]);
					outer_input_sizes.push_back(finished_steps[n_index]->input_sizes[i_index]);
					outer_input_networks.push_back(finished_steps[n_index]->input_networks[i_index]);
				}
				finished_steps[n_index]->input_networks.clear();	// for garbage collection

				scope_score_networks.push_back(finished_steps[n_index]->score_network);
				finished_steps[n_index]->score_network = NULL;	// for garbage collection

				scope_average_scores.push_back(finished_steps[n_index]->average_score);
				scope_average_misguesses.push_back(finished_steps[n_index]->average_misguess);
				scope_average_inner_scope_impacts.push_back(finished_steps[n_index]->average_inner_scope_impact);
				scope_average_local_impacts.push_back(finished_steps[n_index]->average_local_impact);
				scope_average_inner_branch_impacts.push_back(0.0);

				scope_active_compress.push_back(finished_steps[n_index]->active_compress);
				scope_compress_new_sizes.push_back(finished_steps[n_index]->compress_new_size);
				scope_compress_networks.push_back(finished_steps[n_index]->compress_network);
				finished_steps[n_index]->compress_network = NULL;	// for garbage collection
				scope_compress_original_sizes.push_back(finished_steps[n_index]->compress_original_size);

				n_index++;
			} else {
				vector<FinishedStep*> subscope(finished_steps.begin()+scope_starts[s_index],
					finished_steps.begin()+scope_ends[s_index]+1);	// inclusive end

				vector<int> new_outer_input_layer;
				vector<int> new_outer_input_sizes;
				vector<FoldNetwork*> new_outer_input_networks;
				FoldNetwork* new_outer_score_network;
				double new_outer_average_score;
				double new_outer_average_misguess;
				double new_outer_average_local_impact;
				bool new_outer_active_compress;
				int new_outer_compress_new_size;
				FoldNetwork* new_outer_compress_network;
				int new_outer_compress_original_size;
				vector<int> new_compressed_scope_sizes;
				Scope* new_scope = construct_scope_helper(subscope,
														  curr_layer+1,
														  new_outer_input_layer,
														  new_outer_input_sizes,
														  new_outer_input_networks,
														  new_outer_score_network,
														  new_outer_average_score,
														  new_outer_average_misguess,
														  new_outer_average_local_impact,
														  new_outer_active_compress,
														  new_outer_compress_new_size,
														  new_outer_compress_network,
														  new_outer_compress_original_size,
														  new_compressed_scope_sizes);

				scope_sequence_length++;
				scope_is_inner_scope.push_back(true);
				scope_scopes.push_back(new_scope);
				scope_obs_sizes.push_back(-1);

				scope_inner_input_networks.push_back(vector<FoldNetwork*>());
				scope_inner_input_sizes.push_back(vector<int>());
				scope_scope_scale_mod.push_back(1.0);
				for (int i_index = 0; i_index < (int)new_outer_input_networks.size(); i_index++) {
					if (new_outer_input_layer[i_index] == curr_layer) {
						scope_inner_input_networks.back().push_back(new_outer_input_networks[i_index]);
						scope_inner_input_sizes.back().push_back(new_outer_input_sizes[i_index]);
					} else {
						if (new_outer_input_layer[i_index] == curr_layer-1) {
							scope_num_inputs += new_outer_input_sizes[i_index];
						}

						outer_input_layer.push_back(new_outer_input_layer[i_index]);
						outer_input_sizes.push_back(new_outer_input_sizes[i_index]);
						outer_input_networks.push_back(new_outer_input_networks[i_index]);
					}
				}

				scope_step_types.push_back(STEP_TYPE_STEP);
				scope_branches.push_back(NULL);
				scope_folds.push_back(NULL);

				if (scope_ends[s_index] == (int)finished_steps.size()-1) {
					outer_score_network = new_outer_score_network;
					outer_average_score = new_outer_average_score;
					outer_average_misguess = new_outer_average_misguess;
					outer_average_local_impact = new_outer_average_local_impact;
					outer_active_compress = new_outer_active_compress;
					outer_compress_new_size = new_outer_compress_new_size;
					outer_compress_network = new_outer_compress_network;
					outer_compress_original_size = new_outer_compress_original_size;

					scope_num_outputs = new_compressed_scope_sizes.back() + new_scope->num_outputs;
					compressed_scope_sizes = vector<int>(new_compressed_scope_sizes.begin(),
						new_compressed_scope_sizes.end()-1);

					scope_score_networks.push_back(NULL);

					// doesn't matter
					scope_average_scores.push_back(0.0);
					scope_average_misguesses.push_back(0.0);
					scope_average_inner_scope_impacts.push_back(0.0);
					scope_average_local_impacts.push_back(0.0);
					scope_average_inner_branch_impacts.push_back(0.0);

					scope_active_compress.push_back(false);
					scope_compress_new_sizes.push_back(-1);
					scope_compress_networks.push_back(NULL);
					scope_compress_original_sizes.push_back(-1);
				} else {
					scope_score_networks.push_back(new_outer_score_network);

					scope_average_scores.push_back(new_outer_average_score);
					scope_average_misguesses.push_back(new_outer_average_misguess);
					scope_average_inner_scope_impacts.push_back(0.0);	// initialize to 0 (can't simply sum values within as may not be independent)
					scope_average_local_impacts.push_back(new_outer_average_local_impact);
					scope_average_inner_branch_impacts.push_back(0.0);

					scope_active_compress.push_back(new_outer_active_compress);
					scope_compress_new_sizes.push_back(new_outer_compress_new_size);
					scope_compress_networks.push_back(new_outer_compress_network);
					scope_compress_original_sizes.push_back(new_outer_compress_original_size);
				}

				n_index = scope_ends[s_index]+1;	// doesn't matter if scope end

				break;
			}
		}
	}
	while (true) {
		if (n_index >= (int)finished_steps.size()) {
			break;
		}

		scope_sequence_length++;
		scope_is_inner_scope.push_back(finished_steps[n_index]->is_inner_scope);
		scope_scopes.push_back(finished_steps[n_index]->scope);
		finished_steps[n_index]->scope = NULL;	// for garbage collection
		scope_obs_sizes.push_back(finished_steps[n_index]->obs_size);

		scope_inner_input_networks.push_back(vector<FoldNetwork*>());
		scope_inner_input_sizes.push_back(vector<int>());
		scope_scope_scale_mod.push_back(finished_steps[n_index]->scope_scale_mod);
		if (finished_steps[n_index]->is_inner_scope) {
			scope_inner_input_networks.back().push_back(finished_steps[n_index]->inner_input_network);
			finished_steps[n_index]->inner_input_network = NULL;	// for garbage collection
			scope_inner_input_sizes.back().push_back(finished_steps[n_index]->scope->num_inputs);

			for (int i_index = 0; i_index < (int)finished_steps[n_index]->inner_input_input_networks.size(); i_index++) {
				if (finished_steps[n_index]->inner_input_input_layer[i_index] == curr_layer-1) {
					scope_num_inputs += finished_steps[n_index]->inner_input_input_sizes[i_index];
				}

				outer_input_layer.push_back(finished_steps[n_index]->inner_input_input_layer[i_index]);
				outer_input_sizes.push_back(finished_steps[n_index]->inner_input_input_sizes[i_index]);
				outer_input_networks.push_back(finished_steps[n_index]->inner_input_input_networks[i_index]);
			}
		}
		finished_steps[n_index]->inner_input_input_networks.clear();	// for garbage collection

		scope_step_types.push_back(STEP_TYPE_STEP);
		scope_branches.push_back(NULL);
		scope_folds.push_back(NULL);

		for (int i_index = 0; i_index < (int)finished_steps[n_index]->input_networks.size(); i_index++) {
			// note: it might be that for the last step, input networks are only for outer scopes

			if (finished_steps[n_index]->input_layer[i_index] == curr_layer-1) {
				scope_num_inputs += finished_steps[n_index]->input_sizes[i_index];
			}

			outer_input_layer.push_back(finished_steps[n_index]->input_layer[i_index]);
			outer_input_sizes.push_back(finished_steps[n_index]->input_sizes[i_index]);
			outer_input_networks.push_back(finished_steps[n_index]->input_networks[i_index]);
		}
		finished_steps[n_index]->input_networks.clear();	// for garbage collection

		if (n_index == (int)finished_steps.size()-1) {
			scope_score_networks.push_back(NULL);
			outer_score_network = finished_steps[n_index]->score_network;
			finished_steps[n_index]->score_network = NULL;	// for garbage collection

			scope_average_scores.push_back(0.0);	// doesn't matter
			scope_average_misguesses.push_back(finished_steps[n_index]->average_misguess);	// starts identical for both inner and outer
			scope_average_inner_scope_impacts.push_back(finished_steps[n_index]->average_inner_scope_impact);
			scope_average_local_impacts.push_back(0.0);	// doesn't matter
			scope_average_inner_branch_impacts.push_back(0.0);

			outer_average_score = finished_steps[n_index]->average_score;
			outer_average_misguess = finished_steps[n_index]->average_misguess;
			outer_average_local_impact = finished_steps[n_index]->average_local_impact;

			scope_active_compress.push_back(false);
			scope_compress_new_sizes.push_back(-1);
			scope_compress_networks.push_back(NULL);
			scope_compress_original_sizes.push_back(-1);

			outer_active_compress = finished_steps[n_index]->active_compress;
			outer_compress_new_size = finished_steps[n_index]->compress_new_size;
			outer_compress_network = finished_steps[n_index]->compress_network;
			finished_steps[n_index]->compress_network = NULL;	// for garbage collection
			outer_compress_original_size = finished_steps[n_index]->compress_original_size;

			compressed_scope_sizes = finished_steps[n_index]->compressed_scope_sizes;
			scope_num_outputs = compressed_scope_sizes.back();
			compressed_scope_sizes.pop_back();
			scope_num_outputs += compressed_scope_sizes.back();
			compressed_scope_sizes.pop_back();
		} else {
			scope_score_networks.push_back(finished_steps[n_index]->score_network);
			finished_steps[n_index]->score_network = NULL;	// for garbage collection

			scope_average_scores.push_back(finished_steps[n_index]->average_score);
			scope_average_misguesses.push_back(finished_steps[n_index]->average_misguess);
			scope_average_inner_scope_impacts.push_back(finished_steps[n_index]->average_inner_scope_impact);
			scope_average_local_impacts.push_back(finished_steps[n_index]->average_local_impact);
			scope_average_inner_branch_impacts.push_back(0.0);

			scope_active_compress.push_back(finished_steps[n_index]->active_compress);
			scope_compress_new_sizes.push_back(finished_steps[n_index]->compress_new_size);
			scope_compress_networks.push_back(finished_steps[n_index]->compress_network);
			finished_steps[n_index]->compress_network = NULL;	// for garbage collection
			scope_compress_original_sizes.push_back(finished_steps[n_index]->compress_original_size);
		}

		n_index++;
	}

	cout << "scope_sequence_length: " << scope_sequence_length << endl;

	Scope* scope = new Scope(scope_num_inputs,
							 scope_num_outputs,
							 scope_sequence_length,
							 scope_is_inner_scope,
							 scope_scopes,
							 scope_obs_sizes,
							 scope_inner_input_networks,
							 scope_inner_input_sizes,
							 scope_scope_scale_mod,
							 scope_step_types,
							 scope_branches,
							 scope_folds,
							 scope_score_networks,
							 scope_average_scores,
							 scope_average_misguesses,
							 scope_average_inner_scope_impacts,
							 scope_average_local_impacts,
							 scope_average_inner_branch_impacts,
							 scope_active_compress,
							 scope_compress_new_sizes,
							 scope_compress_networks,
							 scope_compress_original_sizes,
							 false);

	return scope;
}

void fold_to_path(vector<FinishedStep*> finished_steps,
				  int& sequence_length,
				  vector<bool>& is_inner_scope,
				  vector<Scope*>& scopes,
				  vector<int>& obs_sizes,
				  vector<vector<FoldNetwork*>>& inner_input_networks,
				  vector<vector<int>>& inner_input_sizes,
				  vector<double>& scope_scale_mod,
				  vector<int>& step_types,
				  vector<Branch*>& branches,
				  vector<Fold*>& folds,
				  vector<FoldNetwork*>& score_networks,
				  vector<double>& average_scores,
				  vector<double>& average_misguesses,
				  vector<double>& average_inner_scope_impacts,
				  vector<double>& average_local_impacts,
				  vector<double>& average_inner_branch_impacts,
				  vector<bool>& active_compress,
				  vector<int>& compress_new_sizes,
				  vector<FoldNetwork*>& compress_networks,
				  vector<int>& compress_original_sizes) {
	vector<int> scope_starts;
	vector<int> scope_ends;

	int curr_start = -1;
	int num_scopes = 1;
	for (int n_index = 0; n_index < (int)finished_steps.size(); n_index++) {
		num_scopes++;
		if (finished_steps[n_index]->compress_num_layers > 0) {
			num_scopes -= finished_steps[n_index]->compress_num_layers;
			if (finished_steps[n_index]->compress_new_size > 0) {
				num_scopes++;
			}

			if (num_scopes == 0) {
				// special case outer base layer
				num_scopes = 1;
			}
		}

		if (num_scopes > 1) {
			if (curr_start == -1) {
				curr_start = n_index;
			}
		} else {
			if (curr_start != -1) {
				scope_starts.push_back(curr_start);
				scope_ends.push_back(n_index);

				curr_start = -1;
			}
		}
	}

	cout << "scopes:" << endl;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		cout << "start: " << scope_starts[s_index] << endl;
		cout << "end: " << scope_ends[s_index] << endl;
	}
	cout << endl;

	sequence_length = 0;

	int n_index = 0;
	for (int s_index = 0; s_index < (int)scope_starts.size(); s_index++) {
		while (true) {
			if (n_index < scope_starts[s_index]) {
				sequence_length++;
				is_inner_scope.push_back(finished_steps[n_index]->is_inner_scope);
				scopes.push_back(finished_steps[n_index]->scope);
				finished_steps[n_index]->scope = NULL;	// for garbage collection
				obs_sizes.push_back(finished_steps[n_index]->obs_size);

				inner_input_networks.push_back(vector<FoldNetwork*>());
				inner_input_sizes.push_back(vector<int>());
				scope_scale_mod.push_back(finished_steps[n_index]->scope_scale_mod);
				if (finished_steps[n_index]->is_inner_scope) {
					inner_input_networks.back().push_back(finished_steps[n_index]->inner_input_network);
					finished_steps[n_index]->inner_input_network = NULL;	// for garbage collection
					inner_input_sizes.back().push_back(finished_steps[n_index]->scope->num_inputs);

					// finished_steps[n_index]->inner_input_input_networks.size() == 0
				}

				step_types.push_back(STEP_TYPE_STEP);
				branches.push_back(NULL);
				folds.push_back(NULL);

				// finished_steps[n_index]->input_networks.size() == 0

				score_networks.push_back(finished_steps[n_index]->score_network);
				finished_steps[n_index]->score_network = NULL;	// for garbage collection

				average_scores.push_back(finished_steps[n_index]->average_score);
				average_misguesses.push_back(finished_steps[n_index]->average_misguess);
				average_inner_scope_impacts.push_back(finished_steps[n_index]->average_inner_scope_impact);
				average_local_impacts.push_back(finished_steps[n_index]->average_local_impact);
				average_inner_branch_impacts.push_back(0.0);

				active_compress.push_back(finished_steps[n_index]->active_compress);
				compress_new_sizes.push_back(finished_steps[n_index]->compress_new_size);
				compress_networks.push_back(finished_steps[n_index]->compress_network);
				finished_steps[n_index]->compress_network = NULL;	// for garbage collection
				compress_original_sizes.push_back(finished_steps[n_index]->compress_original_size);

				n_index++;
			} else {
				vector<FinishedStep*> subscope(finished_steps.begin()+scope_starts[s_index],
					finished_steps.begin()+scope_ends[s_index]+1);	// inclusive end

				vector<int> new_outer_input_layer;
				vector<int> new_outer_input_sizes;
				vector<FoldNetwork*> new_outer_input_networks;
				FoldNetwork* new_outer_score_network;
				double new_outer_average_score;
				double new_outer_average_misguess;
				double new_outer_average_local_impact;
				bool new_outer_active_compress;
				int new_outer_compress_new_size;
				FoldNetwork* new_outer_compress_network;
				int new_outer_compress_original_size;
				vector<int> new_compressed_scope_sizes;
				Scope* new_scope = construct_scope_helper(subscope,
														  1,
														  new_outer_input_layer,
														  new_outer_input_sizes,
														  new_outer_input_networks,
														  new_outer_score_network,
														  new_outer_average_score,
														  new_outer_average_misguess,
														  new_outer_average_local_impact,
														  new_outer_active_compress,
														  new_outer_compress_new_size,
														  new_outer_compress_network,
														  new_outer_compress_original_size,
														  new_compressed_scope_sizes);

				sequence_length++;
				is_inner_scope.push_back(true);
				scopes.push_back(new_scope);
				obs_sizes.push_back(-1);

				inner_input_networks.push_back(vector<FoldNetwork*>());
				inner_input_sizes.push_back(vector<int>());
				scope_scale_mod.push_back(1.0);
				for (int i_index = 0; i_index < (int)new_outer_input_networks.size(); i_index++) {
					// new_outer_input_layer[i_index] == 0
					inner_input_networks.back().push_back(new_outer_input_networks[i_index]);
					inner_input_sizes.back().push_back(new_outer_input_sizes[i_index]);
				}

				step_types.push_back(STEP_TYPE_STEP);
				branches.push_back(NULL);
				folds.push_back(NULL);

				score_networks.push_back(new_outer_score_network);

				average_scores.push_back(new_outer_average_score);
				average_misguesses.push_back(new_outer_average_misguess);
				average_inner_scope_impacts.push_back(0.0);	// initialize to 0 (can't simply sum values within as may not be independent)
				average_local_impacts.push_back(new_outer_average_local_impact);
				average_inner_branch_impacts.push_back(0.0);

				active_compress.push_back(new_outer_active_compress);
				compress_new_sizes.push_back(new_outer_compress_new_size);
				compress_networks.push_back(new_outer_compress_network);
				compress_original_sizes.push_back(new_outer_compress_original_size);

				n_index = scope_ends[s_index]+1;	// doesn't matter if scope end

				break;
			}
		}
	}
	while (true) {
		if (n_index >= (int)finished_steps.size()) {
			break;
		}

		sequence_length++;
		is_inner_scope.push_back(finished_steps[n_index]->is_inner_scope);
		scopes.push_back(finished_steps[n_index]->scope);
		finished_steps[n_index]->scope = NULL;	// for garbage collection
		obs_sizes.push_back(finished_steps[n_index]->obs_size);

		inner_input_networks.push_back(vector<FoldNetwork*>());
		inner_input_sizes.push_back(vector<int>());
		scope_scale_mod.push_back(finished_steps[n_index]->scope_scale_mod);
		if (finished_steps[n_index]->is_inner_scope) {
			inner_input_networks.back().push_back(finished_steps[n_index]->inner_input_network);
			finished_steps[n_index]->inner_input_network = NULL;	// for garbage collection
			inner_input_sizes.back().push_back(finished_steps[n_index]->scope->num_inputs);

			// finished_steps[n_index]->inner_input_input_networks.size() == 0
		}

		step_types.push_back(STEP_TYPE_STEP);
		branches.push_back(NULL);
		folds.push_back(NULL);

		// finished_steps[n_index]->input_networks.size() == 0

		score_networks.push_back(finished_steps[n_index]->score_network);
		finished_steps[n_index]->score_network = NULL;	// for garbage collection

		average_scores.push_back(finished_steps[n_index]->average_score);
		average_misguesses.push_back(finished_steps[n_index]->average_misguess);
		average_inner_scope_impacts.push_back(finished_steps[n_index]->average_inner_scope_impact);
		average_local_impacts.push_back(finished_steps[n_index]->average_local_impact);
		average_inner_branch_impacts.push_back(0.0);

		active_compress.push_back(finished_steps[n_index]->active_compress);
		compress_new_sizes.push_back(finished_steps[n_index]->compress_new_size);
		compress_networks.push_back(finished_steps[n_index]->compress_network);
		finished_steps[n_index]->compress_network = NULL;	// for garbage collection
		compress_original_sizes.push_back(finished_steps[n_index]->compress_original_size);

		n_index++;
	}
}
