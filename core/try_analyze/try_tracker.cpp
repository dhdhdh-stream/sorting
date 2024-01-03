#include "try_tracker.h"

#include "constants.h"
#include "globals.h"
#include "try_analyze_helpers.h"
#include "try_exit_impact.h"
#include "try_impact.h"
#include "try_instance.h"
#include "try_scope_step.h"

using namespace std;

TryTracker::TryTracker() {
	// do nothing
};

TryTracker::TryTracker(ifstream& input_file) {
	string tries_size_line;
	getline(input_file, tries_size_line);
	int tries_size = stoi(tries_size_line);
	for (int t_index = 0; t_index < tries_size; t_index++) {
		this->tries.push_back(new TryInstance(input_file));
	}

	string action_impacts_size_line;
	getline(input_file, action_impacts_size_line);
	int action_impacts_size = stoi(action_impacts_size_line);
	for (int i_index = 0; i_index < action_impacts_size; i_index++) {
		string operation_line;
		getline(input_file, operation_line);
		int operation = stoi(operation_line);

		string action_line;
		getline(input_file, action_line);
		int action = stoi(action_line);

		TryImpact* try_impact = new TryImpact(input_file);

		this->action_impacts[{operation, action}] = try_impact;
	}

	string node_impacts_size_line;
	getline(input_file, node_impacts_size_line);
	int node_impacts_size = stoi(node_impacts_size_line);
	for (int i_index = 0; i_index < node_impacts_size; i_index++) {
		string operation_line;
		getline(input_file, operation_line);
		int operation = stoi(operation_line);

		string parent_id_line;
		getline(input_file, parent_id_line);
		int parent_id = stoi(parent_id_line);

		string node_id_line;
		getline(input_file, node_id_line);
		int node_id = stoi(node_id_line);

		TryImpact* try_impact = new TryImpact(input_file);

		this->node_impacts[{operation, {parent_id, node_id}}] = try_impact;
	}

	string exit_impacts_size_line;
	getline(input_file, exit_impacts_size_line);
	int exit_impacts_size = stoi(exit_impacts_size_line);
	for (int i_index = 0; i_index < exit_impacts_size; i_index++) {
		string operation_line;
		getline(input_file, operation_line);
		int operation = stoi(operation_line);

		string exit_depth_line;
		getline(input_file, exit_depth_line);
		int exit_depth = stoi(exit_depth_line);

		string parent_id_line;
		getline(input_file, parent_id_line);
		int parent_id = stoi(parent_id_line);

		string node_id_line;
		getline(input_file, node_id_line);
		int node_id = stoi(node_id_line);

		TryExitImpact* try_impact = new TryExitImpact(input_file);

		this->exit_impacts[{operation, {exit_depth, {parent_id, node_id}}}] = try_impact;
	}
}

TryTracker::~TryTracker() {
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		delete this->tries[t_index];
	}

	for (map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.begin();
			it != this->action_impacts.end(); it++) {
		delete it->second;
	}
	for (map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts.begin();
			it != this->node_impacts.end(); it++) {
		delete it->second;
	}

	for (map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts.begin();
			it != this->exit_impacts.end(); it++) {
		delete it->second;
	}
}

/**
 * - this->tries.size() != 0
 */
void TryTracker::evaluate_potential(TryInstance* potential,
									double& predicted_impact,
									double& closest_distance) {
	int best_index;
	double best_distance = numeric_limits<double>::max();
	vector<pair<int, pair<int,int>>> best_diffs;
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		double distance;
		vector<pair<int, pair<int,int>>> diffs;
		try_distance(this->tries[t_index],
					 potential,
					 distance,
					 diffs);

		if (this->tries[t_index]->exit != potential->exit) {
			distance += 2;
		}

		if (distance < best_distance) {
			best_index = t_index;
			best_distance = distance;
			best_diffs = diffs;
		}
	}

	double sum_impacts = 0.0;
	for (int d_index = 0; d_index < (int)best_diffs.size(); d_index++) {
		if (best_diffs[d_index].first == TRY_INSERT) {
			if (potential->step_types[best_diffs[d_index].second.second] == STEP_TYPE_ACTION) {
				int action = potential->actions[best_diffs[d_index].second.second];
				map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
				if (it != this->action_impacts.end()) {
					it->second->calc_impact(potential,
											best_diffs[d_index].second.second,
											sum_impacts);
				}
			} else {
				TryScopeStep* try_scope_step = potential->potential_scopes[best_diffs[d_index].second.second];
				for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
					map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
					if (it != this->node_impacts.end()) {
						it->second->calc_impact(potential,
												best_diffs[d_index].second.second,
												sum_impacts);
					}
				}
			}
		} else if (best_diffs[d_index].first == TRY_REMOVE) {
			if (this->tries[best_index]->step_types[best_diffs[d_index].second.first] == STEP_TYPE_ACTION) {
				int action = this->tries[best_index]->actions[best_diffs[d_index].second.first];
				map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
				if (it != this->action_impacts.end()) {
					it->second->calc_impact(this->tries[best_index],
											best_diffs[d_index].second.first,
											sum_impacts);
				}
			} else {
				TryScopeStep* try_scope_step = this->tries[best_index]->potential_scopes[best_diffs[d_index].second.first];
				for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
					map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
						.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
					if (it != this->node_impacts.end()) {
						it->second->calc_impact(this->tries[best_index],
												best_diffs[d_index].second.first,
												sum_impacts);
					}
				}
			}
		} else {
			// best_diffs[d_index].first == TRY_SUBSTITUTE
			vector<pair<int, int>> additions;
			vector<pair<int, int>> removals;
			try_scope_step_diff(this->tries[best_index]->potential_scopes[best_diffs[d_index].second.first],
								potential->potential_scopes[best_diffs[d_index].second.second],
								additions,
								removals);
			for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
				map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
					.find({TRY_INSERT, additions[a_index]});
				if (it != this->node_impacts.end()) {
					it->second->calc_impact(potential,
											best_diffs[d_index].second.second,
											sum_impacts);
				}
			}
			for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
				map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
					.find({TRY_REMOVE, removals[r_index]});
				if (it != this->node_impacts.end()) {
					it->second->calc_impact(this->tries[best_index],
											best_diffs[d_index].second.first,
											sum_impacts);
				}
			}
		}
	}

	if (this->tries[best_index]->exit != potential->exit) {
		{
			map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
				.find({TRY_REMOVE, this->tries[best_index]->exit});
			if (it != this->exit_impacts.end()) {
				it->second->calc_impact(this->tries[best_index],
										sum_impacts);
			}
		}
		{
			map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
				.find({TRY_INSERT, potential->exit});
			if (it != this->exit_impacts.end()) {
				it->second->calc_impact(potential,
										sum_impacts);
			}
		}
	}

	predicted_impact = this->tries[best_index]->result + sum_impacts;

	closest_distance = best_distance;
}

void TryTracker::backprop(TryInstance* new_add) {
	if (this->tries.size() > 0) {
		vector<int> remaining_indexes(this->tries.size());
		for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
			remaining_indexes[t_index] = t_index;
		}

		int num_compare = ceil(log(this->tries.size()));
		for (int c_index = 0; c_index < num_compare; c_index++) {
			uniform_int_distribution<int> distribution(0, remaining_indexes.size()-1);
			int remaining_index = distribution(generator);
			int original_index = remaining_indexes[remaining_index];

			double distance;
			vector<pair<int, pair<int, int>>> diffs;
			try_distance(this->tries[original_index],
						 new_add,
						 distance,
						 diffs);

			int num_impacts = 0;
			double sum_impacts = 0.0;
			for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
				if (diffs[d_index].first == TRY_INSERT) {
					if (new_add->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
						int action = new_add->actions[diffs[d_index].second.second];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
						if (it == this->action_impacts.end()) {
							it = this->action_impacts.insert({{TRY_INSERT, action}, new TryImpact()}).first;
						}
						it->second->calc_impact(new_add,
												diffs[d_index].second.second,
												num_impacts,
												sum_impacts);
					} else {
						TryScopeStep* try_scope_step = new_add->potential_scopes[diffs[d_index].second.second];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
							if (it == this->node_impacts.end()) {
								it = this->node_impacts.insert({{TRY_INSERT, try_scope_step->original_nodes[n_index]}, new TryImpact()}).first;
							}
							it->second->calc_impact(new_add,
													diffs[d_index].second.second,
													num_impacts,
													sum_impacts);
						}
					}
				} else if (diffs[d_index].first == TRY_REMOVE) {
					if (this->tries[original_index]->step_types[diffs[d_index].second.first] == STEP_TYPE_ACTION) {
						int action = this->tries[original_index]->actions[diffs[d_index].second.first];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
						if (it == this->action_impacts.end()) {
							it = this->action_impacts.insert({{TRY_REMOVE, action}, new TryImpact()}).first;
						}
						it->second->calc_impact(this->tries[original_index],
												diffs[d_index].second.first,
												num_impacts,
												sum_impacts);
					} else {
						TryScopeStep* try_scope_step = this->tries[original_index]->potential_scopes[diffs[d_index].second.first];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
							if (it == this->node_impacts.end()) {
								it = this->node_impacts.insert({{TRY_REMOVE, try_scope_step->original_nodes[n_index]}, new TryImpact()}).first;
							}
							it->second->calc_impact(this->tries[original_index],
													diffs[d_index].second.first,
													num_impacts,
													sum_impacts);
						}
					}
				} else {
					// diffs[d_index].first == TRY_SUBSTITUTE
					vector<pair<int, int>> additions;
					vector<pair<int, int>> removals;
					try_scope_step_diff(this->tries[original_index]->potential_scopes[diffs[d_index].second.first],
										new_add->potential_scopes[diffs[d_index].second.second],
										additions,
										removals);
					for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_INSERT, additions[a_index]});
						if (it == this->node_impacts.end()) {
							it = this->node_impacts.insert({{TRY_INSERT, additions[a_index]}, new TryImpact()}).first;
						}
						it->second->calc_impact(new_add,
												diffs[d_index].second.second,
												num_impacts,
												sum_impacts);
					}
					for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_REMOVE, removals[r_index]});
						if (it == this->node_impacts.end()) {
							it = this->node_impacts.insert({{TRY_REMOVE, removals[r_index]}, new TryImpact()}).first;
						}
						it->second->calc_impact(this->tries[original_index],
												diffs[d_index].second.first,
												num_impacts,
												sum_impacts);
					}
				}
			}

			if (this->tries[original_index]->exit != new_add->exit) {
				{
					map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_REMOVE, this->tries[original_index]->exit});
					if (it == this->exit_impacts.end()) {
						it = this->exit_impacts.insert({{TRY_REMOVE, this->tries[original_index]->exit}, new TryExitImpact()}).first;
					}
					it->second->calc_impact(this->tries[original_index],
											num_impacts,
											sum_impacts);
				}
				{
					map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_INSERT, new_add->exit});
					if (it == this->exit_impacts.end()) {
						it = this->exit_impacts.insert({{TRY_INSERT, new_add->exit}, new TryExitImpact()}).first;
					}
					it->second->calc_impact(new_add,
											num_impacts,
											sum_impacts);
				}
			}

			double predicted_impact = this->tries[original_index]->result + sum_impacts;

			double impact_diff = (new_add->result - predicted_impact) / num_impacts;

			for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
				if (diffs[d_index].first == TRY_INSERT) {
					if (new_add->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
						int action = new_add->actions[diffs[d_index].second.second];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
						// it != this->action_impacts.end()
						it->second->backprop(impact_diff,
											 new_add,
											 diffs[d_index].second.second);
					} else {
						TryScopeStep* try_scope_step = new_add->potential_scopes[diffs[d_index].second.second];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
							// it != this->node_impacts.end()
							it->second->backprop(impact_diff,
												 new_add,
												 diffs[d_index].second.second);
						}
					}
				} else if (diffs[d_index].first == TRY_REMOVE) {
					if (this->tries[original_index]->step_types[diffs[d_index].second.first] == STEP_TYPE_ACTION) {
						int action = this->tries[original_index]->actions[diffs[d_index].second.first];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
						// it != this->action_impacts.end()
						it->second->backprop(impact_diff,
											 this->tries[original_index],
											 diffs[d_index].second.first);
					} else {
						TryScopeStep* try_scope_step = this->tries[original_index]->potential_scopes[diffs[d_index].second.first];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
							// it != this->node_impacts.end()
							it->second->backprop(impact_diff,
												 this->tries[original_index],
												 diffs[d_index].second.first);
						}
					}
				} else {
					// diffs[d_index].first == TRY_SUBSTITUTE
					vector<pair<int, int>> additions;
					vector<pair<int, int>> removals;
					try_scope_step_diff(this->tries[original_index]->potential_scopes[diffs[d_index].second.first],
										new_add->potential_scopes[diffs[d_index].second.second],
										additions,
										removals);
					for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_INSERT, additions[a_index]});
						// it != this->node_impacts.end()
						it->second->backprop(impact_diff,
											 new_add,
											 diffs[d_index].second.second);
					}
					for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_REMOVE, removals[r_index]});
						// it != this->node_impacts.end()
						it->second->backprop(impact_diff,
											 this->tries[original_index],
											 diffs[d_index].second.first);
					}
				}
			}

			if (this->tries[original_index]->exit != new_add->exit) {
				{
					map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_REMOVE, this->tries[original_index]->exit});
					// it != this->exit_impacts.end()
					it->second->backprop(impact_diff,
										 this->tries[original_index]);
				}
				{
					map<pair<int, pair<int, pair<int,int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_INSERT, new_add->exit});
					// it != this->exit_impacts.end()
					it->second->backprop(impact_diff,
										 new_add);
				}
			}

			remaining_indexes.erase(remaining_indexes.begin() + remaining_index);
		}
	}

	this->tries.push_back(new_add);
}

void TryTracker::save(ofstream& output_file) {
	output_file << this->tries.size() << endl;
	for (int t_index = 0; t_index < (int)this->tries.size(); t_index++) {
		this->tries[t_index]->save(output_file);
	}

	output_file << this->action_impacts.size() << endl;
	for (map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.begin();
			it != this->action_impacts.end(); it++) {
		output_file << it->first.first << endl;
		output_file << it->first.second << endl;

		it->second->save(output_file);
	}

	output_file << this->node_impacts.size() << endl;
	for (map<pair<int, pair<int, int>>, TryImpact*>::iterator it = this->node_impacts.begin();
			it != this->node_impacts.end(); it++) {
		output_file << it->first.first << endl;
		output_file << it->first.second.first << endl;
		output_file << it->first.second.second << endl;

		it->second->save(output_file);
	}

	output_file << this->exit_impacts.size() << endl;
	for (map<pair<int, pair<int, pair<int, int>>>, TryExitImpact*>::iterator it = this->exit_impacts.begin();
			it != this->exit_impacts.end(); it++) {
		output_file << it->first.first << endl;
		output_file << it->first.second.first << endl;
		output_file << it->first.second.second.first << endl;
		output_file << it->first.second.second.second << endl;

		it->second->save(output_file);
	}
}
