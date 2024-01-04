#include "try_tracker.h"

#include "constants.h"
#include "globals.h"
#include "try_analyze_helpers.h"
#include "try_exit_impact.h"
#include "try_impact.h"
#include "try_instance.h"
#include "try_scope_step.h"
#include "try_start_impact.h"

using namespace std;

TryTracker::TryTracker() {
	// do nothing
};

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

	for (map<pair<int, pair<vector<int>, vector<int>>>, TryStartImpact*>::iterator it = this->start_impacts.begin();
			it != this->start_impacts.end(); it++) {
		delete it->second;
	}

	for (map<pair<int, pair<vector<int>, vector<int>>>, TryExitImpact*>::iterator it = this->exit_impacts.begin();
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

		if (this->tries[t_index]->start != potential->start) {
			distance += 2;
		}
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

	if (this->tries[best_index]->start != potential->start) {
		{
			map<pair<int, pair<vector<int>,vector<int>>>, TryStartImpact*>::iterator it = this->start_impacts
				.find({TRY_REMOVE, this->tries[best_index]->start});
			if (it != this->start_impacts.end()) {
				it->second->calc_impact(this->tries[best_index],
										sum_impacts);
			}
		}
		{
			map<pair<int, pair<vector<int>,vector<int>>>, TryStartImpact*>::iterator it = this->start_impacts
				.find({TRY_INSERT, potential->start});
			if (it != this->start_impacts.end()) {
				it->second->calc_impact(potential,
										sum_impacts);
			}
		}
	}

	if (this->tries[best_index]->exit != potential->exit) {
		{
			map<pair<int, pair<vector<int>,vector<int>>>, TryExitImpact*>::iterator it = this->exit_impacts
				.find({TRY_REMOVE, this->tries[best_index]->exit});
			if (it != this->exit_impacts.end()) {
				it->second->calc_impact(this->tries[best_index],
										sum_impacts);
			}
		}
		{
			map<pair<int, pair<vector<int>,vector<int>>>, TryExitImpact*>::iterator it = this->exit_impacts
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

void TryTracker::update(TryInstance* new_add) {
	if (this->tries.size() > 0) {
		for (int c_index = 0; c_index < (int)this->tries.size(); c_index++) {
			double distance;
			vector<pair<int, pair<int, int>>> diffs;
			try_distance(this->tries[c_index],
						 new_add,
						 distance,
						 diffs);

			int num_impacts = 0;
			for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
				if (diffs[d_index].first == TRY_INSERT) {
					int num_factors = try_step_num_factors(new_add,
														   diffs[d_index].second.second);
					if (new_add->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
						num_impacts += num_factors;
					} else {
						TryScopeStep* try_scope_step = new_add->potential_scopes[diffs[d_index].second.second];
						num_impacts += (int)try_scope_step->original_nodes.size() * num_factors;
					}
				} else if (diffs[d_index].first == TRY_REMOVE) {
					int num_factors = try_step_num_factors(this->tries[c_index],
														   diffs[d_index].second.first);
					if (this->tries[c_index]->step_types[diffs[d_index].second.first] == STEP_TYPE_ACTION) {
						num_impacts += num_factors;
					} else {
						TryScopeStep* try_scope_step = this->tries[c_index]->potential_scopes[diffs[d_index].second.first];
						num_impacts += (int)try_scope_step->original_nodes.size() * num_factors;
					}
				} else {
					// diffs[d_index].first == TRY_SUBSTITUTE
					vector<pair<int, int>> additions;
					vector<pair<int, int>> removals;
					try_scope_step_diff(this->tries[c_index]->potential_scopes[diffs[d_index].second.first],
										new_add->potential_scopes[diffs[d_index].second.second],
										additions,
										removals);
					{
						int num_factors = try_step_num_factors(new_add,
															   diffs[d_index].second.second);
						num_impacts += (int)additions.size() * num_factors;
					}
					{
						int num_factors = try_step_num_factors(this->tries[c_index],
															   diffs[d_index].second.first);
						num_impacts += (int)removals.size() * num_factors;
					}
				}
			}

			if (this->tries[c_index]->start != new_add->start) {
				{
					int num_factors = try_end_num_factors(this->tries[c_index]);
					num_impacts += num_factors;
				}
				{
					int num_factors = try_end_num_factors(new_add);
					num_impacts += num_factors;
				}
			}

			if (this->tries[c_index]->exit != new_add->exit) {
				{
					int num_factors = try_end_num_factors(this->tries[c_index]);
					num_impacts += num_factors;
				}
				{
					int num_factors = try_end_num_factors(new_add);
					num_impacts += num_factors;
				}
			}

			double impact_diff = (new_add->result - this->tries[c_index]->result) / num_impacts;

			for (int d_index = 0; d_index < (int)diffs.size(); d_index++) {
				if (diffs[d_index].first == TRY_INSERT) {
					if (new_add->step_types[diffs[d_index].second.second] == STEP_TYPE_ACTION) {
						int action = new_add->actions[diffs[d_index].second.second];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_INSERT, action});
						if (it == this->action_impacts.end()) {
							it = this->action_impacts.insert({{TRY_INSERT, action}, new TryImpact()}).first;
						}
						it->second->update(impact_diff,
										   new_add,
										   diffs[d_index].second.second);
					} else {
						TryScopeStep* try_scope_step = new_add->potential_scopes[diffs[d_index].second.second];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_INSERT, try_scope_step->original_nodes[n_index]});
							if (it == this->node_impacts.end()) {
								it = this->node_impacts.insert({{TRY_INSERT, try_scope_step->original_nodes[n_index]}, new TryImpact()}).first;
							}
							it->second->update(impact_diff,
											   new_add,
											   diffs[d_index].second.second);
						}
					}
				} else if (diffs[d_index].first == TRY_REMOVE) {
					if (this->tries[c_index]->step_types[diffs[d_index].second.first] == STEP_TYPE_ACTION) {
						int action = this->tries[c_index]->actions[diffs[d_index].second.first];
						map<pair<int, int>, TryImpact*>::iterator it = this->action_impacts.find({TRY_REMOVE, action});
						if (it == this->action_impacts.end()) {
							it = this->action_impacts.insert({{TRY_REMOVE, action}, new TryImpact()}).first;
						}
						it->second->update(impact_diff,
										   this->tries[c_index],
										   diffs[d_index].second.first);
					} else {
						TryScopeStep* try_scope_step = this->tries[c_index]->potential_scopes[diffs[d_index].second.first];
						for (int n_index = 0; n_index < (int)try_scope_step->original_nodes.size(); n_index++) {
							map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
								.find({TRY_REMOVE, try_scope_step->original_nodes[n_index]});
							if (it == this->node_impacts.end()) {
								it = this->node_impacts.insert({{TRY_REMOVE, try_scope_step->original_nodes[n_index]}, new TryImpact()}).first;
							}
							it->second->update(impact_diff,
											   this->tries[c_index],
											   diffs[d_index].second.first);
						}
					}
				} else {
					// diffs[d_index].first == TRY_SUBSTITUTE
					vector<pair<int, int>> additions;
					vector<pair<int, int>> removals;
					try_scope_step_diff(this->tries[c_index]->potential_scopes[diffs[d_index].second.first],
										new_add->potential_scopes[diffs[d_index].second.second],
										additions,
										removals);
					for (int a_index = 0; a_index < (int)additions.size(); a_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_INSERT, additions[a_index]});
						if (it == this->node_impacts.end()) {
							it = this->node_impacts.insert({{TRY_INSERT, additions[a_index]}, new TryImpact()}).first;
						}
						it->second->update(impact_diff,
										   new_add,
										   diffs[d_index].second.second);
					}
					for (int r_index = 0; r_index < (int)removals.size(); r_index++) {
						map<pair<int, pair<int,int>>, TryImpact*>::iterator it = this->node_impacts
							.find({TRY_REMOVE, removals[r_index]});
						if (it == this->node_impacts.end()) {
							it = this->node_impacts.insert({{TRY_REMOVE, removals[r_index]}, new TryImpact()}).first;
						}
						it->second->update(impact_diff,
										   this->tries[c_index],
										   diffs[d_index].second.first);
					}
				}
			}

			if (this->tries[c_index]->start != new_add->start) {
				{
					map<pair<int, pair<vector<int>,vector<int>>>, TryStartImpact*>::iterator it = this->start_impacts
						.find({TRY_REMOVE, this->tries[c_index]->start});
					if (it == this->start_impacts.end()) {
						it = this->start_impacts.insert({{TRY_REMOVE, this->tries[c_index]->start}, new TryStartImpact()}).first;
					}
					it->second->update(impact_diff,
									   this->tries[c_index]);
				}
				{
					map<pair<int, pair<vector<int>,vector<int>>>, TryStartImpact*>::iterator it = this->start_impacts
						.find({TRY_INSERT, new_add->start});
					if (it == this->start_impacts.end()) {
						it = this->start_impacts.insert({{TRY_INSERT, new_add->start}, new TryStartImpact()}).first;
					}
					it->second->update(impact_diff,
									   new_add);
				}
			}

			if (this->tries[c_index]->exit != new_add->exit) {
				{
					map<pair<int, pair<vector<int>,vector<int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_REMOVE, this->tries[c_index]->exit});
					if (it == this->exit_impacts.end()) {
						it = this->exit_impacts.insert({{TRY_REMOVE, this->tries[c_index]->exit}, new TryExitImpact()}).first;
					}
					it->second->update(impact_diff,
									   this->tries[c_index]);
				}
				{
					map<pair<int, pair<vector<int>,vector<int>>>, TryExitImpact*>::iterator it = this->exit_impacts
						.find({TRY_INSERT, new_add->exit});
					if (it == this->exit_impacts.end()) {
						it = this->exit_impacts.insert({{TRY_INSERT, new_add->exit}, new TryExitImpact()}).first;
					}
					it->second->update(impact_diff,
									   new_add);
				}
			}
		}
	}

	this->tries.push_back(new_add);
}
