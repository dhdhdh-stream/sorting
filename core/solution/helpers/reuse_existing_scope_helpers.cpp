#include "solution_helpers.h"

#include "globals.h"
#include "potential_scope_node.h"
#include "scope.h"
#include "scope_node.h"
#include "solution.h"

using namespace std;

PotentialScopeNode* reuse_existing(Problem* problem,
								   vector<ContextLayer>& context,
								   int explore_context_depth) {
	double sum_probabilities = problem->num_actions();
	for (map<int, Scope*>::iterator it = solution->scopes.begin();
			it != solution->scopes.end(); it++) {
		if (it->second->parent_scope_nodes.size() > 0) {
			sum_probabilities += sqrt(it->second->parent_scope_nodes.size());
		}
	}

	uniform_real_distribution<double> scope_distribution(0.0, sum_probabilities);
	double random_probability = scope_distribution(generator);
	if (random_probability < problem->num_actions()) {
		return NULL;
	}
	random_probability -= problem->num_actions();

	Scope* existing_scope;
	for (map<int, Scope*>::iterator it = solution->scopes.begin();
			it != solution->scopes.end(); it++) {
		if (it->second->parent_scope_nodes.size() > 0) {
			random_probability -= sqrt(it->second->parent_scope_nodes.size());
			if (random_probability <= 0.0) {
				existing_scope = it->second;
				break;
			}
		}
	}

	uniform_int_distribution<int> parent_scope_node_distribution(0, existing_scope->parent_scope_nodes.size()-1);
	ScopeNode* existing_scope_node = existing_scope->parent_scope_nodes[parent_scope_node_distribution(generator)];

	PotentialScopeNode* new_potential_scope_node = new PotentialScopeNode();
	new_potential_scope_node->scope = existing_scope;
	new_potential_scope_node->experiment_scope_depth = explore_context_depth;

	uniform_int_distribution<int> matching_include_input_distribution(0, 3);
	uniform_int_distribution<int> non_matching_include_input_distribution(0, 1);
	uniform_int_distribution<int> input_type_distribution(0, 7);
	uniform_real_distribution<double> init_distribution(-1.0, 1.0);
	uniform_int_distribution<int> constant_reuse_distribution(0, 4);
	for (int s_index = 0; s_index < existing_scope->num_input_states; s_index++) {
		int matching_index = -1;
		for (int i_index = 0; i_index < (int)existing_scope_node->input_types.size(); i_index++) {
			if (existing_scope_node->input_inner_indexes[i_index] == s_index) {
				matching_index = i_index;
				break;
			}
		}
		if (matching_index != -1) {
			if (existing_scope_node->input_types[matching_index] == INPUT_TYPE_CONSTANT
					&& constant_reuse_distribution(generator) != 0) {
				new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
				new_potential_scope_node->input_inner_indexes.push_back(s_index);
				new_potential_scope_node->input_scope_depths.push_back(-1);
				new_potential_scope_node->input_outer_is_local.push_back(false);
				new_potential_scope_node->input_outer_indexes.push_back(-1);
				new_potential_scope_node->input_init_vals.push_back(
					existing_scope_node->input_init_vals[matching_index]);
			} else if (matching_include_input_distribution(generator) != 0) {
				int type = input_type_distribution(generator);
				if (type == 0) {
					// constant
					new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_potential_scope_node->input_inner_indexes.push_back(s_index);
					new_potential_scope_node->input_scope_depths.push_back(-1);
					new_potential_scope_node->input_outer_is_local.push_back(false);
					new_potential_scope_node->input_outer_indexes.push_back(-1);
					new_potential_scope_node->input_init_vals.push_back(init_distribution(generator));
				} else if (type == 1) {
					// all state

					vector<int> possible_input_scope_depths;
					vector<bool> possible_input_outer_is_local;
					vector<int> possible_input_outer_indexes;
					{
						for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
								it != context.back().input_state_vals.end(); it++) {
							possible_input_scope_depths.push_back(0);
							possible_input_outer_is_local.push_back(false);
							possible_input_outer_indexes.push_back(it->first);
						}

						for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
								it != context.back().local_state_vals.end(); it++) {
							possible_input_scope_depths.push_back(0);
							possible_input_outer_is_local.push_back(true);
							possible_input_outer_indexes.push_back(it->first);
						}

						/**
						 * - simply don't use temp states as input
						 */
					}
					for (int c_index = 1; c_index < explore_context_depth; c_index++) {
						ScopeNode* scope_node = context[context.size()-1 - c_index].node;

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
								it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == false
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								possible_input_scope_depths.push_back(c_index);
								possible_input_outer_is_local.push_back(false);
								possible_input_outer_indexes.push_back(it->first);
							}
						}

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
								it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == true
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								possible_input_scope_depths.push_back(c_index);
								possible_input_outer_is_local.push_back(true);
								possible_input_outer_indexes.push_back(it->first);
							}
						}
					}

					if (possible_input_scope_depths.size() > 0) {
						uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
						int input_index = input_distribution(generator);

						new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_potential_scope_node->input_inner_indexes.push_back(s_index);
						new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[input_index]);
						new_potential_scope_node->input_outer_is_local.push_back(possible_input_outer_is_local[input_index]);
						new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[input_index]);
						new_potential_scope_node->input_init_vals.push_back(0.0);
					}
				} else {
					// matched state
					/**
					 * - TODO: weigh different degrees of matches
					 */
					int target_state_id = existing_scope->original_input_state_ids[s_index];

					vector<int> possible_input_scope_depths;
					vector<bool> possible_input_outer_is_local;
					vector<int> possible_input_outer_indexes;
					{
						Scope* scope = context.back().scope;

						for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
								it != context.back().input_state_vals.end(); it++) {
							int state_id = scope->original_input_state_ids[it->first];
							if (target_state_id == state_id) {
								possible_input_scope_depths.push_back(0);
								possible_input_outer_is_local.push_back(false);
								possible_input_outer_indexes.push_back(it->first);
							}
						}

						for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
								it != context.back().local_state_vals.end(); it++) {
							int state_id = scope->original_local_state_ids[it->first];
							if (target_state_id == state_id) {
								possible_input_scope_depths.push_back(0);
								possible_input_outer_is_local.push_back(true);
								possible_input_outer_indexes.push_back(it->first);
							}
						}
					}
					for (int c_index = 1; c_index < explore_context_depth; c_index++) {
						Scope* scope = context[context.size()-1 - c_index].scope;
						ScopeNode* scope_node = context[context.size()-1 - c_index].node;

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
								it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == false
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								int state_id = scope->original_input_state_ids[it->first];
								if (target_state_id == state_id) {
									possible_input_scope_depths.push_back(c_index);
									possible_input_outer_is_local.push_back(false);
									possible_input_outer_indexes.push_back(it->first);
								}
							}
						}

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
								it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == true
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								int state_id = scope->original_local_state_ids[it->first];
								if (target_state_id == state_id) {
									possible_input_scope_depths.push_back(c_index);
									possible_input_outer_is_local.push_back(true);
									possible_input_outer_indexes.push_back(it->first);
								}
							}
						}
					}

					if (possible_input_scope_depths.size() > 0) {
						uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
						int input_index = input_distribution(generator);

						new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_potential_scope_node->input_inner_indexes.push_back(s_index);
						new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[input_index]);
						new_potential_scope_node->input_outer_is_local.push_back(possible_input_outer_is_local[input_index]);
						new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[input_index]);
						new_potential_scope_node->input_init_vals.push_back(0.0);
					}
				}
			}
		} else {
			if (non_matching_include_input_distribution(generator) == 0) {
				int type = input_type_distribution(generator);
				if (type == 0) {
					// constant
					new_potential_scope_node->input_types.push_back(INPUT_TYPE_CONSTANT);
					new_potential_scope_node->input_inner_indexes.push_back(s_index);
					new_potential_scope_node->input_scope_depths.push_back(-1);
					new_potential_scope_node->input_outer_is_local.push_back(false);
					new_potential_scope_node->input_outer_indexes.push_back(-1);
					new_potential_scope_node->input_init_vals.push_back(init_distribution(generator));
				} else if (type == 1) {
					// all state

					vector<int> possible_input_scope_depths;
					vector<bool> possible_input_outer_is_local;
					vector<int> possible_input_outer_indexes;
					{
						for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
								it != context.back().input_state_vals.end(); it++) {
							possible_input_scope_depths.push_back(0);
							possible_input_outer_is_local.push_back(false);
							possible_input_outer_indexes.push_back(it->first);
						}

						for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
								it != context.back().local_state_vals.end(); it++) {
							possible_input_scope_depths.push_back(0);
							possible_input_outer_is_local.push_back(true);
							possible_input_outer_indexes.push_back(it->first);
						}

						/**
						 * - simply don't use temp states as input
						 */
					}
					for (int c_index = 1; c_index < explore_context_depth; c_index++) {
						ScopeNode* scope_node = context[context.size()-1 - c_index].node;

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
								it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == false
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								possible_input_scope_depths.push_back(c_index);
								possible_input_outer_is_local.push_back(false);
								possible_input_outer_indexes.push_back(it->first);
							}
						}

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
								it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == true
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								possible_input_scope_depths.push_back(c_index);
								possible_input_outer_is_local.push_back(true);
								possible_input_outer_indexes.push_back(it->first);
							}
						}
					}

					if (possible_input_scope_depths.size() > 0) {
						uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
						int input_index = input_distribution(generator);

						new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_potential_scope_node->input_inner_indexes.push_back(s_index);
						new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[input_index]);
						new_potential_scope_node->input_outer_is_local.push_back(possible_input_outer_is_local[input_index]);
						new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[input_index]);
						new_potential_scope_node->input_init_vals.push_back(0.0);
					}
				} else {
					// matched state
					/**
					 * - TODO: weigh different degrees of matches
					 */
					int target_state_id = existing_scope->original_input_state_ids[s_index];

					vector<int> possible_input_scope_depths;
					vector<bool> possible_input_outer_is_local;
					vector<int> possible_input_outer_indexes;
					{
						Scope* scope = context.back().scope;

						for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
								it != context.back().input_state_vals.end(); it++) {
							int state_id = scope->original_input_state_ids[it->first];
							if (target_state_id == state_id) {
								possible_input_scope_depths.push_back(0);
								possible_input_outer_is_local.push_back(false);
								possible_input_outer_indexes.push_back(it->first);
							}
						}

						for (map<int, StateStatus>::iterator it = context.back().local_state_vals.begin();
								it != context.back().local_state_vals.end(); it++) {
							int state_id = scope->original_local_state_ids[it->first];
							if (target_state_id == state_id) {
								possible_input_scope_depths.push_back(0);
								possible_input_outer_is_local.push_back(true);
								possible_input_outer_indexes.push_back(it->first);
							}
						}
					}
					for (int c_index = 1; c_index < explore_context_depth; c_index++) {
						Scope* scope = context[context.size()-1 - c_index].scope;
						ScopeNode* scope_node = context[context.size()-1 - c_index].node;

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
								it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == false
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								int state_id = scope->original_input_state_ids[it->first];
								if (target_state_id == state_id) {
									possible_input_scope_depths.push_back(c_index);
									possible_input_outer_is_local.push_back(false);
									possible_input_outer_indexes.push_back(it->first);
								}
							}
						}

						for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].local_state_vals.begin();
								it != context[context.size()-1 - c_index].local_state_vals.end(); it++) {
							bool passed_down = false;
							for (int i_index = 0; i_index < (int)scope_node->input_types.size(); i_index++) {
								if (scope_node->input_outer_is_local[i_index] == true
										&& scope_node->input_outer_indexes[i_index] == it->first) {
									passed_down = true;
									break;
								}
							}

							if (!passed_down) {
								int state_id = scope->original_local_state_ids[it->first];
								if (target_state_id == state_id) {
									possible_input_scope_depths.push_back(c_index);
									possible_input_outer_is_local.push_back(true);
									possible_input_outer_indexes.push_back(it->first);
								}
							}
						}
					}

					if (possible_input_scope_depths.size() > 0) {
						uniform_int_distribution<int> input_distribution(0, possible_input_scope_depths.size()-1);
						int input_index = input_distribution(generator);

						new_potential_scope_node->input_types.push_back(INPUT_TYPE_STATE);
						new_potential_scope_node->input_inner_indexes.push_back(s_index);
						new_potential_scope_node->input_scope_depths.push_back(possible_input_scope_depths[input_index]);
						new_potential_scope_node->input_outer_is_local.push_back(possible_input_outer_is_local[input_index]);
						new_potential_scope_node->input_outer_indexes.push_back(possible_input_outer_indexes[input_index]);
						new_potential_scope_node->input_init_vals.push_back(0.0);
					}
				}
			}
		}
	}

	/**
	 * - assign outputs in random order
	 */
	vector<int> possible_output_scope_depths;
	vector<bool> possible_output_outer_is_local;
	vector<int> possible_output_outer_indexes;
	vector<int> possible_output_types;
	{
		Scope* scope = context.back().scope;

		/**
		 * - check for OuterExperiment edge case
		 */
		if (scope != NULL) {
			uniform_int_distribution<int> include_output_distribution(0, 1);

			for (map<int, StateStatus>::iterator it = context.back().input_state_vals.begin();
					it != context.back().input_state_vals.end(); it++) {
				if (include_output_distribution(generator) == 0) {
					possible_output_scope_depths.push_back(0);
					possible_output_outer_is_local.push_back(false);
					possible_output_outer_indexes.push_back(it->first);
					possible_output_types.push_back(scope->original_input_state_ids[it->first]);
				}
			}

			for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
				if (include_output_distribution(generator) == 0) {
					possible_output_scope_depths.push_back(0);
					possible_output_outer_is_local.push_back(true);
					possible_output_outer_indexes.push_back(s_index);
					possible_output_types.push_back(scope->original_local_state_ids[s_index]);
				}
			}
		}
	}
	for (int c_index = 1; c_index < explore_context_depth; c_index++) {
		Scope* scope = context[context.size()-1 - c_index].scope;
		ScopeNode* scope_node = context[context.size()-1 - c_index].node;

		uniform_int_distribution<int> include_output_distribution(0, 1 + c_index);

		for (map<int, StateStatus>::iterator it = context[context.size()-1 - c_index].input_state_vals.begin();
				it != context[context.size()-1 - c_index].input_state_vals.end(); it++) {
			if (include_output_distribution(generator) == 0) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == false
							&& scope_node->output_outer_indexes[o_index] == it->first) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_is_local.push_back(false);
					possible_output_outer_indexes.push_back(it->first);
					possible_output_types.push_back(scope->original_input_state_ids[it->first]);
				}
			}
		}

		for (int s_index = 0; s_index < scope->num_local_states; s_index++) {
			if (include_output_distribution(generator) == 0) {
				bool passed_out = false;
				for (int o_index = 0; o_index < (int)scope_node->output_inner_indexes.size(); o_index++) {
					if (scope_node->output_outer_is_local[o_index] == true
							&& scope_node->output_outer_indexes[o_index] == s_index) {
						passed_out = true;
						break;
					}
				}

				if (!passed_out) {
					possible_output_scope_depths.push_back(c_index);
					possible_output_outer_is_local.push_back(true);
					possible_output_outer_indexes.push_back(s_index);
					possible_output_types.push_back(scope->original_local_state_ids[s_index]);
				}
			}
		}
	}

	uniform_int_distribution<int> state_mismatch_distribution(0, 5);
	for (int p_index = 0; p_index < (int)possible_output_scope_depths.size(); p_index++) {
		vector<int> possible_state_indexes;
		if (state_mismatch_distribution(generator) == 0) {
			for (int s_index = 0; s_index < existing_scope->num_input_states; s_index++) {
				possible_state_indexes.push_back(s_index);
			}
		} else {
			for (int s_index = 0; s_index < existing_scope->num_input_states; s_index++) {
				if (existing_scope->original_input_state_ids[s_index] == possible_output_types[p_index]) {
					possible_state_indexes.push_back(s_index);
				}
			}
		}
		if (possible_state_indexes.size() > 0) {
			uniform_int_distribution<int> distribution(0, possible_state_indexes.size()-1);
			int state_index = possible_state_indexes[distribution(generator)];

			new_potential_scope_node->output_inner_indexes.push_back(state_index);
			new_potential_scope_node->output_scope_depths.push_back(possible_output_scope_depths[p_index]);
			new_potential_scope_node->output_outer_is_local.push_back(possible_output_outer_is_local[p_index]);
			new_potential_scope_node->output_outer_indexes.push_back(possible_output_outer_indexes[p_index]);
		}
	}

	return new_potential_scope_node;
}
