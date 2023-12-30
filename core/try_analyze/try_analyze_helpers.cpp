#include "try_analyze_helpers.h"

double try_scope_step_cost(TryScopeStep* first,
						   TryScopeStep* second) {
	vector<AbstractNode*> shared;
	for (int f_index = 0; f_index < (int)first->original_nodes.size(); f_index++) {
		bool is_match = false;
		for (int s_index = 0; s_index < (int)second->original_nodes.size(); s_index++) {
			if (first->original_nodes[f_index] == second->original_nodes[s_index]) {
				is_match = true;
				break;
			}
		}

		if (is_match) {
			shared.push_back(first->original_nodes[f_index]);
		}
	}

	set<AbstractNode*> all;
	for (int f_index = 0; f_index < (int)first->original_nodes.size(); f_index++) {
		all.insert(first->original_nodes[f_index]);
	}
	for (int s_index = 0; s_index < (int)second->original_nodes.size(); s_index++) {
		all.insert(second->original_nodes[s_index]);
	}

	return ((double)shared.size())/((double)all.size());
}

void try_distance(TryInstance* original,
				  TryInstance* potential,
				  double& distance,
				  vector<pair<int, pair<int, int>>>& diffs) {
	int original_length = (int)original->step_types.size();
	int potential_length = (int)potential->step_types.size();

	vector<vector<double>> try_scope_step_costs(original_length, vector<double>(potential_length));
	for (int o_index = 0; o_index < original_length; o_index++) {
		if (original->step_types[o_index] == STEP_TYPE_POTENTIAL_SCOPE) {
			for (int p_index = 0; p_index < potential_length; p_index++) {
				if (potential->step_types[p_index] == STEP_TYPE_POTENTIAL_SCOPE) {
					try_scope_step_costs[o_index][p_index] = try_scope_step_cost(
						original->potential_scopes[o_index],
						potential->potential_scopes[p_index]);
				}
			}
		}
	}

	vector<vector<double>> d(original_length+1, vector<int>(potential_length+1));
	vector<vector<vector<pair<int, pair<int, int>>>>> d_diffs(original_length+1, vector<vector<pair<int, pair<int, int>>>>(potential_length+1));

	d[0][0] = 0.0;
	for (int o_index = 1; o_index < original_length+1; o_index++) {
		d[o_index][0] = o_index;
		d_diffs[o_index][0] = d_diffs[o_index-1][0];
		d_diffs[o_index][0].push_back({TRY_REMOVE, {o_index-1, 0}});
	}
	for (int p_index = 1; p_index < potential_length+1; p_index++) {
		d[0][p_index] = p_index;
		d_diffs[0][p_index] = d_diffs[0][p_index-1];
		d_diffs[0][p_index].push_back({TRY_INSERT, {0, p_index-1}});
	}

	for (int o_index = 1; o_index < original_length+1; o_index++) {
		for (int p_index = 1; p_index < potential_length+1; p_index++) {
			double substitution_cost;
			if (original->step_types[o_index-1] == STEP_TYPE_POTENTIAL_SCOPE
					&& potential->step_types[p_index-1] == STEP_TYPE_POTENTIAL_SCOPE) {
				substitution_cost = d[o_index-1][p_index-1]
					+ 2.0 * (1.0 - try_scope_step_costs[o_index-1][p_index-1]);
			} else if (original->step_types[o_index-1] == STEP_TYPE_ACTION
					&& potential->step_types[p_index-1] == STEP_TYPE_ACTION
					&& original->actions[o_index-1] == potential->actions[p_index-1]) {
				substitution_cost = d[o_index-1][p_index-1];
			} else {
				substitution_cost = numeric_limits<double>::max();
			}
			double insertion_cost = d[o_index][p_index-1] + 1.0;
			double deletion_cost = d[o_index-1][p_index] + 1.0;

			if (substitution_cost < insertion_cost
					&& substitution_cost < deletion_cost) {
				d[o_index][p_index] = substitution_cost;
				d_diffs[o_index][p_index] = d_diffs[o_index-1][p_index-1];
				if (original->step_types[o_index-1] == STEP_TYPE_POTENTIAL_SCOPE
						&& potential->step_types[p_index-1] == STEP_TYPE_POTENTIAL_SCOPE) {
					d_diffs[o_index][p_index].push_back({TRY_SUBSTITUTE, {o_index-1, p_index-1}});
				}
			} else if (insertion < deletion) {
				d[o_index][p_index] = insertion_cost;
				d_diffs[o_index][p_index] = d_diffs[o_index][p_index-1];
				d_diffs[o_index][p_index].push_back({TRY_INSERT, {o_index-1, p_index-1}});
			} else {
				d[o_index][p_index] = deletion_cost;
				d_diffs[o_index][p_index] = d_diffs[o_index-1][p_index];
				d_diffs[o_index][p_index].push_back({TRY_REMOVE, {o_index-1, p_index-1}});
			}
		}
	}

	distance = d.back().back();
	diffs = d_diffs.back().back();
}
